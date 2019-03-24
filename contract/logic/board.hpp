#pragma once

#include <algorithm>  // std::min
#include <string>

#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <eosio/time.hpp>
#include <eosio/crypto.hpp>

namespace logic {
static const uint8_t BOARD_WIDTH = 5;
static const uint8_t TILES_SIZE = BOARD_WIDTH * BOARD_WIDTH;
enum tile : uint8_t {
  UNKNOWN,
  ATTACK_UNREVEALED,
  ATTACK_MISS,
  ATTACK_SHIP1,
  ATTACK_SHIP2,
  ATTACK_SHIP3
};

static const std::map<tile, uint8_t> ship_attacks_map = {
    {ATTACK_SHIP1, 2}, {ATTACK_SHIP2, 1}, {ATTACK_SHIP3, 1}};

static const void assert_no_duplicate_ships(const std::vector<uint8_t> &v) {
  // create a copy of vector and sort it
  std::vector<uint8_t> sorted = v;
  std::sort(sorted.begin(), sorted.end());

  // skip all tiles less than ships
  auto it = sorted.begin();
  while (*it < ATTACK_SHIP1 && it != sorted.end()) it++;

  // consecutive elements must now be different
  while (it != sorted.end() && (it + 1) != sorted.end()) {
    eosio::check(*it != *(it + 1), "duplicate ships");
    it++;
  }
}

// a board stores (hidden) board information for a single player,
// i.e., the opposing player's attacks and our announced results
struct board {
  // is used as vector<tile>, but cannot be serialized by EOS
  std::vector<uint8_t> tiles;
  eosio::checksum256 commitment;

  board() : board(eosio::checksum256()) {}

  board(eosio::checksum256 commitment) : commitment(commitment) {
    for (int i = 0; i < TILES_SIZE; i++) {
      tiles.emplace_back(UNKNOWN);
    }
  }

  EOSLIB_SERIALIZE(board, (tiles)(commitment))

  uint8_t get_max_attacks_amount() const {
    uint8_t attacks = 0;
    for (const std::pair<tile, uint8_t> &pair : ship_attacks_map) {
      attacks += pair.second;
    }
    return attacks;
  }

  // calculates max attacks for the player by subtracting destroyed ships'
  // attacks
  uint8_t get_attacks_amount() const {
    uint8_t attacks = get_max_attacks_amount();

    std::for_each(tiles.begin(), tiles.end(), [&](const uint8_t &t) {
      if (t == ATTACK_SHIP1 || t == ATTACK_SHIP2 || t == ATTACK_SHIP3) {
        attacks -= ship_attacks_map.at((tile)t);
      }
    });
    return attacks;
  }

  bool has_ships() const { return get_attacks_amount() > 0; }

  void attack(const std::vector<uint8_t> &attacks,
              const board &attacker_board) {
    int attacks_amount = attacker_board.get_attacks_amount();
    int unknown_tiles_amount =
        std::count_if(tiles.begin(), tiles.end(),
                      [&](const uint8_t &tile) { return tile == UNKNOWN; });
    int expected_attacks = std::min(attacks_amount, unknown_tiles_amount);

    // always do max attacks, except if there are not enough tiles to attack
    // left
    eosio::check(attacks.size() == expected_attacks,
                 "incorrect amount of attacks");

    std::for_each(attacks.begin(), attacks.end(), [&](uint8_t tile_index) {
      eosio::check(tiles[tile_index] == UNKNOWN, "tile already attacked");
      tiles[tile_index] = ATTACK_UNREVEALED;
    });
  }

  // attack_responses is an array of (a part of) tile enums indicating hit or
  // miss sorted from the lowest to highest index of the tiles to be revealed
  void reveal(const std::vector<uint8_t> &attack_responses) {
    std::for_each(
        attack_responses.begin(), attack_responses.end(), [&](uint8_t r) {
          eosio::check(
              r == ATTACK_MISS || r == ATTACK_SHIP1 || r == ATTACK_SHIP2 ||
                  r == ATTACK_SHIP3,
              "invalid attack response. must be 'miss' or a ship type");
        });
    logic::assert_no_duplicate_ships(attack_responses);

    int reveal_counter = 0;
    std::for_each(tiles.begin(), tiles.end(), [&](uint8_t &tile) {
      if (tile == ATTACK_UNREVEALED) {
        eosio::check(reveal_counter < attack_responses.size(),
                     "must reveal all unrevealed attacks");
        tile = attack_responses[reveal_counter++];
      }
    });

    eosio::check(reveal_counter == attack_responses.size(),
                 "tried to reveal more attacks than existant");
  }

  // this function gets called when the game is over and verifies that the
  // player 1) provides a valid decommitment such that SHA256(decommitment) =
  // commitment 2) announced the attack responses correctly
  void decommit(const eosio::checksum256 &decommitment) const {
    const auto d_bytes = decommitment.extract_as_byte_array();
    assert_sha256((const char *)d_bytes.data(), d_bytes.size(), commitment);
    std::vector<uint8_t> revealed_ship_indexes = {d_bytes[0], d_bytes[1],
                                                  d_bytes[2]};

    const char *assert_message = "caught cheating";

    // check revealed_ship_indexes for validity
    for (int i = 0; i < 3; i++) {
      uint8_t ship_index = revealed_ship_indexes[i];
      // ship_index must be in range
      eosio::check(ship_index < TILES_SIZE, assert_message);

      uint8_t announced_tile = tiles[ship_index];
      // announced tile must not have been revealed yet or match the correct
      // ship
      eosio::check(announced_tile == UNKNOWN ||
                       announced_tile == ATTACK_UNREVEALED ||
                       announced_tile == (ATTACK_SHIP1 + i),
                   assert_message);
    }

    // assert other indexes were not announced as hits
    for (auto index = 0; index < TILES_SIZE; index++) {
      uint8_t t = tiles[index];

      // the revealed_ship_indexes have already been checked for validity
      if (std::find(revealed_ship_indexes.begin(), revealed_ship_indexes.end(),
                    index) != revealed_ship_indexes.end()) {
        continue;
      }

      // every other tile must now either not have been revealed yet or
      // announced as a miss
      eosio::check(t == UNKNOWN || t == ATTACK_UNREVEALED || t == ATTACK_MISS,
                   assert_message);
    }

    // passed all checks
  }
};

}  // namespace logic