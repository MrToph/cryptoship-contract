#include <eosiolib/transaction.hpp>
#include "./utils.hpp"
#include "../cryptoship.hpp"

void send_amount_deferred(eosio::name _self, uint64_t game_id, eosio::name to, const eosio::asset &quantity)
{
    eosio::transaction t{};
    t.actions.emplace_back(
        permission_level{_self, "active"_n},
        "eosio.token"_n,
        "transfer"_n,
        make_tuple(_self, to, quantity, std::to_string(game_id)));

    // set delay in seconds
    t.delay_sec = 0;

    // first argument is a unique sender id
    // second argument is account paying for RAM
    t.send(game_id, _self);
}