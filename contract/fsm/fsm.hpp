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

    state get_winner()
    {
        eosio_assert(data.state == P1_WIN || data.state == P2_WIN || data.state == DRAW, "not in a regular winning state");
        return (state)data.state;
    };

    bool is_in_end_state()
    {
        switch (data.state)
        {
        case P1_WIN:
        case P2_WIN:
        case DRAW:
        case P1_WIN_EXPIRED:
        case P2_WIN_EXPIRED:
        case NEVER_STARTED:
        {
            return true;
        }
        default:
            return false;
        }
    };

    void expire_game(bool *p1_can_claim, bool *p2_can_claim)
    {
        switch (data.state)
        {
        // no money was deposited
        case CREATED:
        {
            data.state = NEVER_STARTED;
            *p1_can_claim = false;
            *p2_can_claim = false;
            break;
        }

        // only P1 deposited and no P2 joined, full return
        case P1_DEPOSITED:
        {
            data.state = NEVER_STARTED;
            *p1_can_claim = true;
            *p2_can_claim = false;
            break;
        }

        // all states where it's P2's turn
        case ALL_DEPOSITED:
        case P2_REVEALED:
        case P1_REVEALED:
        {
            data.state = P1_WIN_EXPIRED;
            *p1_can_claim = true;
            *p2_can_claim = false;
            break;
        }

        // all states where it's P1's turn
        case P2_ATTACKED:
        case P1_ATTACKED:
        case P2_VERIFIED:
        {
            data.state = P2_WIN_EXPIRED;
            *p1_can_claim = false;
            *p2_can_claim = true;
            break;
        }

        // all other states are already end states
        default:
        {
            eosio_assert(false, "game already in an end state");
        }
        }
    }

    uint32_t get_payout_multiplier()
    {
        switch (data.state)
        {
        case NEVER_STARTED:
        {
            // note that we get to this state also from CREATED
            // where P1 did not transfer the funds yet
            // however then he is p1_can_claim is set to false
            return 1;
        }

        case P1_WIN:
        case P1_WIN_EXPIRED:
        {
            return 2;
        }

        case P2_WIN:
        case P2_WIN_EXPIRED:
        {
            return 2;
        }

        case DRAW:
        {
            return 1;
        }
        // all other states are not end states
        default:
        {
            eosio_assert(false, "game is not in an end state - no claims possible yet");
            return 0; // make compiler happy
        }
        }
    }

    void expire_game(bool *p1_won, uint32_t *multiplier);
    void p1_deposit();
    void p2_deposit();
    void join(const eosio::checksum256 &commitment);
    void attack(bool is_player1, const std::vector<uint8_t> &attacks);
    void reveal(bool is_player1, const std::vector<uint8_t> &attack_responses);
    void decommit(bool is_player1, const eosio::checksum256 &decommitment);

    game_data data;
};
} // namespace fsm