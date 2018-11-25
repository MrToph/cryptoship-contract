#pragma once

#include <string>
#include <algorithm> // std::min

#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/time.hpp>

namespace logic
{
static const uint8_t BOARD_WIDTH = 5;
static const uint8_t TILES_SIZE = BOARD_WIDTH * BOARD_WIDTH;
enum tile : uint8_t
{
    UNKNWON,
    ATTACK_UNREVEALED,
    ATTACK_MISS,
    ATTACK_SHIP1,
    ATTACK_SHIP2,
    ATTACK_SHIP3
};
static const std::map<tile, uint8_t> ship_attacks_map = {
    {ATTACK_SHIP1, 2},
    {ATTACK_SHIP2, 1},
    {ATTACK_SHIP3, 1}};

// a board stores (hidden) board information for a single player,
// i.e., the opposing player's attacks and their announced results
struct board
{
    // is used as vector<tile>, but cannot be serialized by EOS
    std::vector<uint8_t> tiles;
    eosio::checksum256 commitment;

    board() : board(eosio::checksum256())
    {
    }

    board(eosio::checksum256 commitment) : commitment(commitment)
    {
        for (int i = 0; i < TILES_SIZE; i++)
        {
            tiles.emplace_back(UNKNWON);
        }
    }

    tile get_xy(uint8_t row, uint8_t column) const
    {
        return (tile)tiles[row * BOARD_WIDTH + column];
    }

    uint8_t get_max_attacks_amount() const
    {
        uint8_t attacks = 0;
        for (const std::pair<tile, uint8_t> &pair : ship_attacks_map)
        {
            attacks += pair.second;
        }
        return attacks;
    }

    // calculates max attacks for the player by subtracting destroyed ships' attacks
    uint8_t get_attacks_amount()
    {
        uint8_t attacks = get_max_attacks_amount();

        std::for_each(tiles.begin(), tiles.end(), [&](const uint8_t &t) {
            if (t == ATTACK_SHIP1 || t == ATTACK_SHIP2 || t == ATTACK_SHIP3)
            {
                attacks -= ship_attacks_map.at((tile)t);
            }
        });
        return attacks;
    }

    bool has_ships() {
        return get_attacks_amount() > 0;
    }

    // attack_responses is an array of integers in range 0,1,2,3
    // each representing miss or the type of the hit ship
    void reveal(const std::vector<uint8_t> &attack_responses)
    {
        std::for_each(attack_responses.begin(), attack_responses.end(), [&](uint8_t r) {
            eosio_assert(r == ATTACK_MISS || r == ATTACK_SHIP1 || r == ATTACK_SHIP2 || r == ATTACK_SHIP3,
                         "invalid attack response. must be 'miss' or a ship type");
        });

        int i = 0;
        std::for_each(tiles.begin(), tiles.end(), [&](uint8_t &tile) {
            if (tile == ATTACK_UNREVEALED)
            {
                eosio_assert(i < attack_responses.size(), "must reveal all unrevealed attacks");
                tile = attack_responses[i++];
            }
        });

        eosio_assert(i == attack_responses.size(), "tried to reveal more attacks than existant");
    }

    void attack(const std::vector<uint8_t> &attacks)
    {
        int attacks_amount = get_attacks_amount();
        int unknown_tiles_amount = std::count_if(tiles.begin(), tiles.end(), [&](const uint8_t &tile) {
            return tile == UNKNWON;
        });
        int expected_attacks = std::min(attacks_amount, unknown_tiles_amount);

        // always do max attacks, except if there are not enough tiles to attack left
        eosio_assert(attacks.size() == expected_attacks, "incorrect amount of attacks");

        std::for_each(attacks.begin(), attacks.end(), [&](uint8_t tile_index) {
            eosio_assert(tiles[tile_index] == UNKNWON, "tile already attacked");
            tiles[tile_index] = ATTACK_UNREVEALED;
        });
    }

    // this function gets called when the game is over and verifies that the player
    // 1) announced the attack responses correctly
    // 2) placed the correct amount of ships
    bool decommit_and_verify(const uint8_t revealed_ship_indexes[3])
    {
        // assert SHA256(revealed_ships) == commitment

        // check revealed_ship_indexes for validity
        for (int i = 0; i < 3; i++)
        {
            int shipIndex = revealed_ship_indexes[i];
            uint8_t announced_tile = tiles[shipIndex];
            // shipIndex must be in range and announced tile must be one of unknown, unrevealed or the correct ship
            if (shipIndex >= TILES_SIZE ||
                !(announced_tile == UNKNWON || announced_tile == ATTACK_UNREVEALED || announced_tile == (ATTACK_SHIP1 + i)))
                return false;
        }

        // assert other indexes were not announced hits
        for (auto index = 0; index < TILES_SIZE; index++)
        {
            uint8_t t = tiles[index];
            // the revealed_ship_indexes have already been checked for validity
            if (index == revealed_ship_indexes[0] || index == revealed_ship_indexes[1] || index == revealed_ship_indexes[2])
                continue;

            // every other tile must now be either unknown, unrevealed, or announced as a miss
            if (!(t == UNKNWON || t == ATTACK_UNREVEALED || t == ATTACK_MISS))
            {
                return false;
            }
        }

        // passed all checks
        return true;
    }

    EOSLIB_SERIALIZE(board, (tiles)(commitment))
};

} // namespace logic