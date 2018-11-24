#pragma once

#include <string>

#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/time.hpp>

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
    // winning states
    CREATED,
    P1_WIN,
    P2_WIN,
    DRAW,
    // won by opponent taking no action in time
    P1_WIN_EXPIRED,
    P2_WIN_EXPIRED,
};

struct game_data
{
    logic::board board1;
    logic::board board2;
    state state;
};

class automaton
{
  public:
    automaton()
        : data() {}
    automaton(const game_data &gd)
        : data(gd) {}

    void create_game();
    void join_game();

    game_data data;
};
} // namespace fsm