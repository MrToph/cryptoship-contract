#include "./cryptoship.hpp"

// https://eosio.stackexchange.com/a/1349/118
#include "./cleanup/cleanup.cpp"
#include "./fsm/fsm.cpp"

using namespace eosio;
using namespace std;

void cryptoship::init()
{
    require_auth(_self);
}

// create just "opens" the game by allocating and paying for RAM
void cryptoship::create(name player, const asset quantity)
{
    require_auth(player);
    // any step between 0.1 and 100 EOS
    eosio_assert(quantity.amount == 1E3 || quantity.amount == 1E4 || quantity.amount == 1E5 || quantity.amount == 1E6, "Must pay any of 0.1 / 1.0 / 10.0 / 100.0 EOS");

    // default constructor initializes game data correctly
    fsm::automaton machine;
    // make player pay for RAM
    games.emplace(player, [&](game &g) {
        // auto-increment key
        g.id = games.available_primary_key();
        g.player1 = player;
        g.bet_amount_per_player = quantity;
        g.expires_at = time_point_sec(now() + EXPIRE_OPEN);
        g.game_data = machine.data;
    });
}

void cryptoship::create_game_deposit(name player, const asset &quantity)
{
    require_auth(player);
    // this action should be called in a transaction after the "create" action
    // only then we can guarantee that the last created game is the opened game
    auto latest_game = games.end();
    latest_game--;

    eosio_assert(latest_game != games.end(), "must create a game first");
    eosio_assert(latest_game->player1 == player, "must pay for your own game");
    eosio_assert(latest_game->bet_amount_per_player == quantity, "game has a different bet amount");

    fsm::automaton machine(latest_game->game_data);
    machine.create_game_deposit();

    games.modify(latest_game, player, [&](game &g) {
        g.expires_at = time_point_sec(now() + EXPIRE_TURN);
        g.game_data = machine.data;
    });
}

void cryptoship::join_game(name player, uint64_t game_id, const asset &quantity)
{
    require_auth(player);

    const auto game = games.find(game_id);
    eosio_assert(game != games.end(), "Game not found");
    eosio_assert(game->bet_amount_per_player == quantity, "game has a different bet amount");

    fsm::automaton machine(game->game_data);
    machine.join_game();

    // cannot make second player pay for updates as this is in a require_recipient call from transfer
    games.modify(game, game->player1, [&](auto &g) {
        g.player2 = player;
        g.expires_at = time_point_sec(now() + EXPIRE_OPEN);
        g.game_data = machine.data;
    });
}

void cryptoship::transfer(name from, name to, const asset& quantity, string memo)
{
    if (from == _self)
    {
        // we're sending money, do nothing additional
        return;
    }

    eosio_assert(to == _self, "contract is not involved in this transfer");
    eosio_assert(quantity.symbol.is_valid(), "invalid quantity");
    eosio_assert(quantity.amount > 0, "only positive quantity allowed");
    eosio_assert(quantity.symbol == EOS_SYMBOL, "only EOS tokens allowed");

    if (memo == "create")
    {
        create_game_deposit(from, quantity);
    }
    else
    {
        uint64_t game_id = std::stoull(memo);
        join_game(from, game_id, quantity);
    }
}

void cryptoship::reveal(uint64_t game_id, eosio::name player, const std::vector<uint8_t> &attack_responses)
{ 
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

void cryptoship::attack(uint64_t game_id, eosio::name player, const std::vector<uint8_t> &attacks)
{

}

void cryptoship::testreset()
{
    require_auth(_self);
    auto itr = games.begin();
    while (itr != games.end())
    {
        itr = games.erase(itr);
    }
}

extern "C" void apply(uint64_t receiver, uint64_t code, uint64_t action)
{
    if (code == "eosio.token"_n.value && action == "transfer"_n.value)
    {
        eosio::execute_action(eosio::name(receiver), eosio::name(code), &cryptoship::transfer);
    }
    else if (code == receiver)
    {
        switch (action)
        {
            EOSIO_DISPATCH_HELPER(cryptoship,
                                  (create)(reveal)(attack)(init)(cleanup)
#ifndef PRODUCTION
                                      (testreset)
#endif
            )
        }
    }
}
