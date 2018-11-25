#pragma once

#include <string>

#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/time.hpp>

#include "../board/board.hpp"

namespace fsm
{
// finite state machine states
enum state : uint8_t
{
    // running game states
    P1_REVEALED,
    P1_ATTACKED,
    P2_REVEALED,
    P2_ATTACKED,
    P2_VERIFIED,

    // game over game states
    CREATED,
    P1_DEPOSITED,
    ALL_DEPOSITED,
    // winning states
    P1_WIN,
    P2_WIN,
    DRAW,
    // won by opponent taking no action in time
    P1_WIN_EXPIRED,
    P2_WIN_EXPIRED,
    NEVER_STARTED,
};

struct game_data
{
    // default constructor needed for multi_index default initialization
    game_data() : board1(), board2(), state(CREATED) {}
    game_data(const eosio::checksum256 &commitment) : board1(commitment), board2(), state(CREATED) {}
    // is used as state struct, but cannot be serialized by EOS
    uint8_t state;
    logic::board board1;
    logic::board board2;

    EOSLIB_SERIALIZE(game_data, (state)(board1)(board2))
};

class automaton
{
  public:
    automaton(const eosio::checksum256 &commitment)
        : data(commitment) {}
    automaton(const game_data &gd)
        : data(gd) {}

    state get_state() { return (state)data.state; };

    void p1_deposit();
    void p2_deposit();
    void join(const eosio::checksum256 &commitment);
    void attack(bool is_player1, const std::vector<uint8_t> &attacks);
    void reveal(bool is_player1, const std::vector<uint8_t> &attack_responses);
    void decommit(bool is_player1, const eosio::checksum256 &decommitment);

    game_data data;
};
} // namespace fsm