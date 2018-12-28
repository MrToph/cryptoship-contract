#include "./utils.hpp"
#include <eosiolib/transaction.hpp>
#include "../cryptoship.hpp"

void claim_deferred(eosio::name _self, uint64_t game_id, eosio::name player) {
  eosio::transaction t{};
  t.actions.emplace_back(permission_level{_self, "active"_n}, _self, "claim"_n,
                         std::make_tuple(game_id, player));

  // set delay in seconds
  t.delay_sec = 0;

  // first argument is a unique sender id
  // second argument is account paying for RAM
  uint128_t sender_id = (((uint128_t)game_id) << 64) + player.value;
  t.send(sender_id, _self);
}