#include "./cryptoship.hpp"

// https://eosio.stackexchange.com/a/1349/118
#include "./cleanup/cleanup.cpp"
#include "./fsm/fsm.cpp"
#include "./utils/utils.cpp"

using namespace eosio;
using namespace std;

// create just "opens" the game by allocating and paying for RAM
void cryptoship::create(name player, uint32_t nonce, const asset quantity,
                        const eosio::checksum256 &commitment) {
  require_auth(player);
  // any step between 0.1 and 100 EOS
  eosio_assert(quantity.amount == 1E3 || quantity.amount == 1E4 ||
                   quantity.amount == 1E5 || quantity.amount == 1E6,
               "Must pay any of 0.1 / 1.0 / 10.0 / 100.0 EOS");

  // default constructor initializes game data correctly
  fsm::automaton machine(commitment);
  // make player pay for RAM
  games.emplace(player, [&](game &g) {
    // auto-increment key
    g.id = games.available_primary_key();
    g.player1 = player;
    g.player1_nonce = nonce;
    g.player1_can_claim = false;
    g.player2_can_claim = false;
    g.bet_amount_per_player = quantity;
    g.expires_at = time_point_sec(now() + EXPIRE_OPEN);
    g.game_data = machine.data;
  });
}

void cryptoship::join(eosio::name player, uint32_t nonce, uint64_t game_id,
                      const eosio::checksum256 &commitment) {
  require_auth(player);

  auto game_itr = get_game(game_id);
  eosio_assert(game_itr->player2 == player,
               "deposit of another player already exists");

  fsm::automaton machine(game_itr->game_data);
  machine.join(commitment);

  games.modify(game_itr, game_itr->player1, [&](auto &g) {
    g.expires_at = time_point_sec(now() + EXPIRE_TURN);
    g.player2_nonce = nonce;
    g.game_data = machine.data;
  });
}

void cryptoship::p1_deposit(name player, const asset &quantity) {
  require_auth(player);
  // this action should be called in a transaction after the "create" action
  // only then we can guarantee that the last created game is the opened game
  auto latest_game = games.end();
  latest_game--;

  eosio_assert(latest_game != games.end(), "must create a game first");
  eosio_assert(latest_game->player1 == player, "must pay for your own game");
  eosio_assert(latest_game->bet_amount_per_player == quantity,
               "game has a different bet amount or token");

  fsm::automaton machine(latest_game->game_data);
  machine.p1_deposit();

  games.modify(latest_game, player, [&](game &g) {
    g.expires_at = time_point_sec(now() + EXPIRE_OPEN);
    g.game_data = machine.data;
  });
}

void cryptoship::p2_deposit(name player, uint64_t game_id,
                            const asset &quantity) {
  require_auth(player);

  const auto game = games.find(game_id);
  eosio_assert(game != games.end(), "Game not found");
  eosio_assert(game->bet_amount_per_player == quantity,
               "game has a different bet amount");
  eosio_assert(game->player1 != player, "cannot join your own game");

  fsm::automaton machine(game->game_data);
  machine.p2_deposit();

  // cannot make second player pay for updates as this is in a require_recipient
  // call from transfer
  games.modify(game, game->player1, [&](auto &g) {
    g.player2 = player;
    g.expires_at = time_point_sec(now() + EXPIRE_OPEN);
    g.game_data = machine.data;
  });
}

void cryptoship::transfer(name from, name to, const asset &quantity,
                          string memo) {
  if (from == _self) {
    // we're sending money, do nothing additional
    return;
  }

  eosio_assert(to == _self, "contract is not involved in this transfer");
  eosio_assert(quantity.symbol.is_valid(), "invalid quantity");
  eosio_assert(quantity.amount > 0, "only positive quantity allowed");
  eosio_assert(quantity.symbol == EOS_SYMBOL, "only EOS tokens allowed");

  if (memo == "create") {
    p1_deposit(from, quantity);
  } else {
    uint64_t game_id = std::stoull(memo);
    p2_deposit(from, game_id, quantity);
  }
}

void cryptoship::attack(uint64_t game_id, eosio::name player,
                        const std::vector<uint8_t> &attacks) {
  require_auth(player);
  auto game_itr = get_game(game_id);
  assert_player_in_game(*game_itr, player);

  fsm::automaton machine(game_itr->game_data);
  machine.attack(player == game_itr->player1, attacks);

  games.modify(game_itr, game_itr->player1, [&](auto &g) {
    g.expires_at = time_point_sec(now() + EXPIRE_TURN);
    g.game_data = machine.data;
  });
}

void cryptoship::reveal(uint64_t game_id, eosio::name player,
                        const std::vector<uint8_t> &attack_responses) {
  require_auth(player);
  auto game_itr = get_game(game_id);
  assert_player_in_game(*game_itr, player);

  fsm::automaton machine(game_itr->game_data);
  machine.reveal(player == game_itr->player1, attack_responses);

  games.modify(game_itr, game_itr->player1, [&](auto &g) {
    g.expires_at = time_point_sec(now() + EXPIRE_TURN);
    g.game_data = machine.data;
  });
}

void cryptoship::decommit(uint64_t game_id, eosio::name player,
                          const eosio::checksum256 &decommitment) {
  require_auth(player);
  auto game_itr = get_game(game_id);
  assert_player_in_game(*game_itr, player);

  bool is_player1 = player == game_itr->player1;
  fsm::automaton machine(game_itr->game_data);
  machine.decommit(is_player1, decommitment);

  games.modify(game_itr, game_itr->player1, [&](auto &g) {
    // player1 decommits means that the game is over and a winner was determined
    g.expires_at =
        time_point_sec(now() + (is_player1 ? EXPIRE_GAME_OVER : EXPIRE_TURN));
    g.game_data = machine.data;
  });

  // player1 decommits means that the game is over and a winner was determined
  if (is_player1) {
    switch (machine.get_winner()) {
      case P1_WIN: {
        action(permission_level{_self, "active"_n}, "eosio.token"_n,
               "transfer"_n,
               make_tuple(_self, game_itr->player1,
                          game_itr->bet_amount_per_player * 2,
                          std::to_string(game_itr->id)))
            .send();
        break;
      }
      case P2_WIN: {
        action(permission_level{_self, "active"_n}, "eosio.token"_n,
               "transfer"_n,
               make_tuple(_self, game_itr->player2,
                          game_itr->bet_amount_per_player * 2,
                          std::to_string(game_itr->id)))
            .send();
        break;
      }
      case DRAW: {
        game_itr = get_game(game_id);
        games.modify(game_itr, game_itr->player1, [&](auto &g) {
          g.player1_can_claim = true;
          g.player2_can_claim = true;
        });
        // send deferred because P2 has incentive to block transfers
        // as he gets 2*stake if this action fails
        claim_deferred(_self, game_itr->id, game_itr->player1);
        claim_deferred(_self, game_itr->id, game_itr->player2);
        break;
      }
      default: {
        eosio_assert(false, "FSM is in a broken state");
      }
    }
  }
}

void cryptoship::claim(uint64_t game_id, eosio::name player) {
  auto game_itr = get_game(game_id);
  assert_player_in_game(*game_itr, player);

  bool is_player1 = player == game_itr->player1;
  bool can_claim =
      is_player1 ? game_itr->player1_can_claim : game_itr->player2_can_claim;
  eosio_assert(can_claim, "not allowed to claim");

  fsm::automaton machine(game_itr->game_data);
  auto multiplier = machine.get_payout_multiplier();

  games.modify(game_itr, game_itr->player1, [&](auto &g) {
    if (is_player1) {
      g.player1_can_claim = false;
    } else {
      g.player2_can_claim = false;
    }
  });

  action(permission_level{_self, "active"_n}, "eosio.token"_n, "transfer"_n,
         make_tuple(_self, player, game_itr->bet_amount_per_player * multiplier,
                    std::to_string(game_itr->id)))
      .send();
}

void cryptoship::testreset() {
  require_auth(_self);
  auto itr = games.begin();
  while (itr != games.end()) {
    itr = games.erase(itr);
  }
}

extern "C" void apply(uint64_t receiver, uint64_t code, uint64_t action) {
  if (code == "eosio.token"_n.value && action == "transfer"_n.value) {
    eosio::execute_action(eosio::name(receiver), eosio::name(code),
                          &cryptoship::transfer);
  } else if (code == receiver) {
    switch (action) {
      EOSIO_DISPATCH_HELPER(
          cryptoship, (create)(join)(attack)(reveal)(decommit)(claim)(cleanup)
#ifndef PRODUCTION
                          (testreset)
#endif
      )
    }
  }
}
