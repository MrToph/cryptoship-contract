// #include "../cryptoship.hpp"
#include "./fsm.hpp"

using namespace fsm;

void automaton::join_game()
{
    eosio_assert(data.state == DEPOSITED, "game has already started");
    data.state = P2_REVEALED;
}

void automaton::create_game_deposit()
{
    eosio_assert(data.state == CREATED, "game has already started");
    data.state = DEPOSITED;
}

void automaton::reveal(bool is_player1, const std::vector<uint8_t> &attack_responses) {
    if(is_player1) {
        eosio_assert(data.state == P1_ATTACKED, "P1 must attack first");
        data.state = P1_REVEALED;
    } else {
        eosio_assert(data.state == P1_REVEALED, "P1 must reveal first");
        data.state = P2_REVEALED;
    }

    (is_player1 ? data.board1 : data.board2).reveal(attack_responses);
}

void automaton::attack(bool is_player1, const std::vector<uint8_t> &attacks) {
    if(is_player1) {
        eosio_assert(data.state == P2_ATTACKED, "P2 must attack first");
        data.state = P1_ATTACKED;
    } else {
        eosio_assert(data.state == P2_REVEALED, "P2 must reveal first");
        bool game_over = !data.board1.has_ships() || !data.board2.has_ships();
        eosio_assert(!game_over, "The game is already in an end state. You must decommit");
        data.state = P2_ATTACKED;
    }

    // player1 attacks are marked on board2 and vice versa
    (is_player1 ? data.board2 : data.board1).attack(attacks);
}
