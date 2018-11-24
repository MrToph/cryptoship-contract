// #include "../cryptoship.hpp"
#include "./fsm.hpp"

using namespace fsm;

void automaton::join_game()
{
    eosio_assert(data.state == CREATED, "game has already started");
    data.state = P2_REVEALED;
}
