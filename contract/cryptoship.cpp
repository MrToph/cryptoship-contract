#include "./cryptoship.hpp"

// https://eosio.stackexchange.com/a/1349/118
#include "./cleanup/cleanup.cpp"

using namespace eosio;
using namespace std;

void cryptoship::init()
{
    require_auth(_self);
}

void cryptoship::create_game(name player, asset quantity)
{
    require_auth(player);
    // any step between 0.1 and 100 EOS
    eosio_assert(quantity.amount == 1E3 || quantity.amount == 1E4 || quantity.amount == 1E5 || quantity.amount == 1E6, "Must pay any of 0.1 / 1.0 / 10.0 / 100.0 EOS");

    // make player pay for RAM
    games.emplace(player, [&](game &g) {
        // auto-increment key
        g.id = games.available_primary_key();
        g.player1 = player;
        g.bet_amount_per_player = quantity;
        g.status = OPEN;
        g.expires_at = time_point_sec(now() + EXPIRE_OPEN);
        g.board1 = logic::board{};
    });
}

void cryptoship::join_game(name player, uint64_t game_id, asset quantity)
{
    
}

void cryptoship::transfer(name from, name to, asset quantity, string memo)
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

    if(memo == "create") {
        create_game(from, quantity);
    }
    else {
        uint64_t game_id = std::stoull(memo);
        const auto game = games.find(game_id);
        eosio_assert(game != games.end() && game->status == OPEN, "Game not found or already started");
        join_game(from, game_id, quantity);
    }
}

void cryptoship::turn(uint64_t game_id, eosio::name player, std::vector<uint8_t> &attack_responses, std::vector<uint8_t> &attacks) {

}

void cryptoship::testreset()
{
    require_auth(_self);
    auto itr = games.begin();
    while(itr != games.end()) {
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
                (turn)
                (init)
                (cleanup)
                #ifndef PRODUCTION
                (testreset)
                #endif
            )
        }
    }
}
