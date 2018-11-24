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
    DEPOSITED,
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
    game_data() : board1(), board2(), state(CREATED) {}
    // is used as state struct, but cannot be serialized by EOS
    uint8_t state;
    logic::board board1;
    logic::board board2;

    EOSLIB_SERIALIZE(game_data, (state)(board1)(board2))
};

class automaton
{
  public:
    automaton()
        : data() {}
    automaton(const game_data &gd)
        : data(gd) {}

    void join_game();
    void create_game_deposit();

    game_data data;
};
} // namespace fsm