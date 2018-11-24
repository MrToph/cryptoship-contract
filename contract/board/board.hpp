#pragma once

#include <string>

#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/time.hpp>

namespace logic
{
static const uint8_t BOARD_SIZE = 5;
static const uint8_t TILES_ARR_SIZE = BOARD_SIZE * BOARD_SIZE;
enum tile : uint8_t
{
    UNKNWON,
    ATTACK_UNREVEALED,
    ATTACK_MISS,
    ATTACK_SHIP1,
    ATTACK_SHIP2,
    ATTACK_SHIP3
};
static const std::map<tile, uint8_t> ship_shots_map = {
    {ATTACK_SHIP1, 2},
    {ATTACK_SHIP2, 1},
    {ATTACK_SHIP3, 1}};

// a board stores (hidden) board information for a single player,
// i.e., the opposing player's attacks and their announced results
struct board
{
    tile tiles[TILES_ARR_SIZE];
    capi_checksum256 commitment;

    tile getXY(uint8_t row, uint8_t column) const
    {
        return tiles[row * BOARD_SIZE + column];
    }

    uint8_t getMaxNumberOfShots() const {
        uint8_t shots = 0;
        for (const std::pair<tile, uint8_t> &pair : ship_shots_map)
        {
            shots += pair.second;
        }
        return shots;
    }

    // calculates max shots for the player by subtracting destroyed ships' shots
    uint8_t getNumberOfShots()
    {
        uint8_t shots = getMaxNumberOfShots();
        
        for (auto t : tiles)
        {
            if (t == ATTACK_SHIP1 || t == ATTACK_SHIP2 || t == ATTACK_SHIP3)
            {
                shots -= ship_shots_map.at(t);
            }
        }
        return shots;
    }

    // this function gets called when the game is over and verifies that the player
    // 1) announced the attack responses correctly
    // 2) placed the correct amount of ships
    bool reveal_and_verify(const uint8_t revealed_ship_indexes[3])
    {
        // assert SHA256(revealed_ships) == commitment

        // check revealed_ship_indexes for validity
        for (int i = 0; i < 3; i++)
        {
            int shipIndex = revealed_ship_indexes[i];
            tile announced_tile = tiles[shipIndex];
            // shipIndex must be in range and announced tile must be one of unknown, unrevealed or the correct ship
            if (shipIndex >= TILES_ARR_SIZE ||
                !(announced_tile == UNKNWON || announced_tile == ATTACK_UNREVEALED || announced_tile == (ATTACK_SHIP1 + i)))
                return false;
        }

        // assert other indexes were not announced hits
        for (auto index = 0; index < TILES_ARR_SIZE; index++)
        {
            tile t = tiles[index];
            // the revealed_ship_indexes have already been checked for validity
            if (index == revealed_ship_indexes[0] || index == revealed_ship_indexes[1] || index == revealed_ship_indexes[2]) continue;

            // every other tile must now be either unknown, unrevealed, or announced as a miss 
            if (!(t == UNKNWON || t == ATTACK_UNREVEALED || t == ATTACK_MISS))
            {
                return false;
            }
        }

        // passed all checks
        return true;
    }

    EOSLIB_SERIALIZE(board, (tiles))
};

} // namespace logic