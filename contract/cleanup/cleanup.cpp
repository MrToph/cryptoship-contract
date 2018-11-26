#include <eosiolib/transaction.hpp>
#include "../cryptoship.hpp"
#include "../utils/utils.hpp"

using namespace eosio;

static const uint64_t CLEANUP_PERIOD_IN_SECONDS = 60 * 5;
// should never interfere with a uint64_t game_id
static const uint128_t CLEANUP_SENDER_ID = -1;

// used to create cleanup action payload
struct cleanup_s {};

// This action is invoked by deferred transactions
// Its purpose is to clean "advance" games that are expired
// for instance by setting expired running games to game over
// or deleting finished games after their expiry date
void cryptoship::cleanup()
{
    // anyone can call this action, no auth required

    auto games_by_expiry = games.get_index<"expiresat"_n>();
    uint8_t count = 3;

    // iterate through all expired games
    auto upper = games_by_expiry.upper_bound(now());
    for (auto game_itr = games_by_expiry.begin(); count > 0 && game_itr != upper; count--)
    {
        fsm::automaton machine(game_itr->game_data);

        // 1. either game was already in an end state => it's time to free RAM
        if (machine.is_in_end_state())
        {
            game_itr = games_by_expiry.erase(game_itr);
        }
        // 2. or a player did not respond => move to an end state and pay out
        else
        {
            bool player1_won;
            uint32_t multiplier;
            machine.expire_game(&player1_won, &multiplier);
            games_by_expiry.modify(game_itr, game_itr->player1, [&](auto &g) {
                g.expires_at = time_point_sec(now() + EXPIRE_GAME_OVER);
                g.game_data = machine.data;
            });
            if (multiplier != 0)
            {
                send_amount_deferred(
                    _self, game_itr->id,
                    player1_won ? game_itr->player1 : game_itr->player2,
                    multiplier * game_itr->bet_amount_per_player);
            }
            game_itr++;
        }
    }

    // schedule the next cleanup
    eosio::transaction t{};
    t.actions.emplace_back(
        permission_level{_self, "active"_n},
        _self,
        "cleanup"_n,
        cleanup_s{}
    );
    t.delay_sec = CLEANUP_PERIOD_IN_SECONDS;
    t.send(CLEANUP_SENDER_ID, _self, true);
}
