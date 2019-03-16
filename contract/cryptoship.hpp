#pragma once

#include <string>
#include <vector>

#include <eosiolib/asset.hpp>
#include <eosiolib/eosio.hpp>
#include <eosiolib/time.hpp>

#include "fsm/fsm.hpp"

#define EOS_SYMBOL symbol("EOS", 4)

// #define PRODUCTION

// 7 days
static const uint32_t EXPIRE_OPEN = 60 * 60 * 24 * 7;
// 1 day
static const uint32_t EXPIRE_TURN = 60 * 60 * 24 * 1;
// 3 days
static const uint32_t EXPIRE_GAME_OVER = 60 * 60 * 24 * 3;

CONTRACT cryptoship : public eosio::contract {
 public:
  cryptoship(eosio::name receiver, eosio::name code,
             eosio::datastream<const char *> ds)
      : contract(receiver, code, ds), games(receiver, receiver.value) {}

  TABLE game {
    // game meta information
    uint64_t id;
    eosio::name player1;
    eosio::name player2;
    // needed for hashing from the frontend when hiding ships
    // (pk id is not a nonce, it can repeat when erasing games)
    uint32_t player1_nonce;
    uint32_t player2_nonce;
    // when payouts are done with deferred transactions and they fail
    // this flag handles the alternative manual payout
    bool player1_can_claim;
    bool player2_can_claim;
    eosio::asset bet_amount_per_player;
    eosio::time_point_sec expires_at;
    // actual game data like ships, hits, etc.
    fsm::game_data game_data;

    auto primary_key() const { return id; }
    uint64_t by_expires_at() const { return expires_at.sec_since_epoch(); }
    uint64_t by_player1() const { return player1.value; }
    uint64_t by_player2() const { return player2.value; }
    uint64_t by_game_state() const { return game_data.state; }

    EOSLIB_SERIALIZE(game, (id)(player1)(player2)(player1_nonce)(player2_nonce)(
                               player1_can_claim)(player2_can_claim)(
                               bet_amount_per_player)(expires_at)(game_data))
  };

  typedef eosio::multi_index<
      "games"_n, game,
      eosio::indexed_by<
          "player1"_n, eosio::const_mem_fun<game, uint64_t, &game::by_player1>>,
      eosio::indexed_by<
          "player2"_n, eosio::const_mem_fun<game, uint64_t, &game::by_player2>>,
      eosio::indexed_by<
          "expiresat"_n,
          eosio::const_mem_fun<game, uint64_t, &game::by_expires_at>>,
      eosio::indexed_by<"state"_n, eosio::const_mem_fun<game, uint64_t,
                                                        &game::by_game_state>>>
      games_t;

  auto get_game(uint64_t game_id) {
    const auto game = games.find(game_id);
    eosio_assert(game != games.end(), "Game not found");
    return game;
  }

  void assert_player_in_game(const game &game, eosio::name player) {
    eosio_assert(game.player1 == player || game.player2 == player,
                 "You are not part of this game");
  }

#ifndef PRODUCTION
  ACTION testreset(uint16_t max_games);
#endif
  // implemented in cleanup.cpp
  ACTION cleanup();
  ACTION create(eosio::name player, uint32_t nonce, const eosio::asset quantity,
                const eosio::checksum256 &commitment);
  ACTION join(eosio::name player, uint32_t nonce, uint64_t game_id,
              const eosio::checksum256 &commitment);
  ACTION attack(uint64_t game_id, eosio::name player,
                const std::vector<uint8_t> &attacks);
  ACTION reveal(uint64_t game_id, eosio::name player,
                const std::vector<uint8_t> &attack_responses);
  ACTION decommit(uint64_t game_id, eosio::name player,
                  const eosio::checksum256 &decommitment);
  ACTION claim(uint64_t game_id, eosio::name player);

  void transfer(eosio::name from, eosio::name to, const eosio::asset &quantity,
                std::string memo);
  void p1_deposit(eosio::name player, const eosio::asset &quantity);
  void p2_deposit(eosio::name player, uint64_t game_id,
                  const eosio::asset &quantity);

  games_t games;
};
