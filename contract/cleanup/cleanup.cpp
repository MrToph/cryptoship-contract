#include <eosiolib/transaction.hpp>
#include "../cryptoship.hpp"
#include "../utils/utils.hpp"

using namespace eosio;

static const uint64_t CLEANUP_PERIOD_IN_SECONDS = 60 * 5;
// should never interfere with a uint64_t game_id
static const uint128_t CLEANUP_SENDER_ID = -1;

// used to create cleanup action payload
struct cleanup_s {};

// This action is invoked by deferred transactions
// Its purpose is to "advance" games that are expired
// for instance by setting expired running games to game over
// or deleting finished games after their expiry date
void cryptoship::cleanup() {
  // anyone can call this action, no auth required

  auto games_by_expiry = games.get_index<"expiresat"_n>();
  uint8_t count = 3;

  // store up to count expired games
  auto upper_bound = games_by_expiry.upper_bound(now());
  std::vector<uint64_t> expired_ids;
  for (auto game_itr = games_by_expiry.begin();
       count > 0 && game_itr != upper_bound; count--, game_itr++) {
    expired_ids.push_back(game_itr->id);
  }

  // iterate over these games and advance them
  for (uint64_t id : expired_ids) {
    auto game_itr = games.find(id);

    if (game_itr == games.end()) {
      continue;
    }
    fsm::automaton machine(game_itr->game_data);

    // 1. either game was already in an end state => it's time to free RAM
    if (machine.is_in_end_state()) {
      games.erase(game_itr);
    }
    // 2. or a player did not respond => move to an end state and allow payouts
    else {
      bool player1_can_claim = false;
      bool player2_can_claim = false;
      machine.expire_game(&player1_can_claim, &player2_can_claim);

      games.modify(game_itr, game_itr->player1, [&](auto &g) {
        g.expires_at = time_point_sec(now() + EXPIRE_GAME_OVER);
        g.game_data = machine.data;
        g.player1_can_claim = player1_can_claim;
        g.player2_can_claim = player2_can_claim;
      });

      // when a game is expired at most one of the players can get a payout (no
      // draw)
      if (player1_can_claim) {
        claim_deferred(_self, game_itr->id, game_itr->player1);
      }
      if (player2_can_claim) {
        claim_deferred(_self, game_itr->id, game_itr->player2);
      }
    }
  }

  // schedule the next cleanup
  eosio::transaction t{};
  t.actions.emplace_back(permission_level{_self, "active"_n}, _self,
                         "cleanup"_n, cleanup_s{});
  t.delay_sec = CLEANUP_PERIOD_IN_SECONDS;
  // cannot replace transactions right now
  // https://github.com/EOSIO/eos/issues/6541
  // t.send(CLEANUP_SENDER_ID, _self, true);
  cancel_deferred(CLEANUP_SENDER_ID);
  t.send(CLEANUP_SENDER_ID, _self, false);
}
