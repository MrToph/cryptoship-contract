#pragma once

#include <string>
#include <vector>

#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/time.hpp>

#include "board/board.hpp"
#include "fsm/fsm.hpp"

#define EOS_SYMBOL symbol("EOS", 4)

// #define PRODUCTION

// 7 days
static const uint32_t EXPIRE_OPEN = 60 * 60 * 24 * 7;
// 1 day
static const uint32_t EXPIRE_TURN = 60 * 60 * 24 * 1;
// 3 days
static const uint32_t EXPIRE_GAME_OVER = 60 * 60 * 24 * 3;

CONTRACT cryptoship : public eosio::contract
{
    public:
    cryptoship(eosio::name receiver, eosio::name code, eosio::datastream<const char *> ds)
      : contract(receiver, code, ds), games(receiver, receiver.value) {}

    TABLE game {
        // game meta information
        uint64_t id;
        eosio::name player1;
        eosio::name player2;
        eosio::asset bet_amount_per_player;
        eosio::time_point_sec expires_at;
        // actual game data like ships, hits, etc.
        fsm::game_data game_data;

        auto primary_key() const { return id; }
        uint64_t by_expires_at() const { return expires_at.sec_since_epoch(); }
        uint64_t by_player1() const { return player1.value; }
        uint64_t by_player2() const { return player2.value; }
        uint64_t by_game_state() const { return game_data.state; }

        EOSLIB_SERIALIZE(game, (id)(player1)(player2)(bet_amount_per_player)(expires_at)(game_data))
    };

    typedef eosio::multi_index<
      "games"_n,
      game,
      eosio::indexed_by<"player1"_n, eosio::const_mem_fun<game, uint64_t, &game::by_player1>>,
      eosio::indexed_by<"player2"_n, eosio::const_mem_fun<game, uint64_t, &game::by_player2>>,
      eosio::indexed_by<"expiresat"_n, eosio::const_mem_fun<game, uint64_t, &game::by_expires_at>>
      >
      games_t;

    void create_game_deposit(eosio::name player, const eosio::asset& quantity);
    void join_game(eosio::name player, uint64_t game_id, const eosio::asset& quantity);

    #ifndef PRODUCTION
    ACTION testreset();
    #endif
    ACTION init();
    ACTION create(eosio::name player, const eosio::asset quantity);
    ACTION cleanup();
    ACTION turn(uint64_t game_id, eosio::name player, std::vector<uint8_t> &attack_responses, std::vector<uint8_t> &attacks);
    void transfer(eosio::name from, eosio::name to, eosio::asset quantity, std::string memo);

    games_t games;
};
