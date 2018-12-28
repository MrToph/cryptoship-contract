// #include "../cryptoship.hpp"
#include "./fsm.hpp"

using namespace fsm;

void automaton::p1_deposit() {
  eosio_assert(data.state == CREATED, "player1 funds already deposited");
  data.state = P1_DEPOSITED;
}

void automaton::p2_deposit() {
  eosio_assert(data.state == P1_DEPOSITED, "player2 cannot deposit now");
  data.state = ALL_DEPOSITED;
}

void automaton::join(const eosio::checksum256 &commitment) {
  eosio_assert(data.state == ALL_DEPOSITED,
               "player2 needs to deposit first or game already started");
  data.board2.commitment = commitment;
  data.state = P2_REVEALED;
}

void automaton::attack(bool is_player1, const std::vector<uint8_t> &attacks) {
  if (is_player1) {
    eosio_assert(data.state == P2_ATTACKED, "P2 must attack first");
    data.state = P1_ATTACKED;
  } else {
    eosio_assert(data.state == P2_REVEALED, "P2 must reveal first");
    bool game_over = !data.board1.has_ships() || !data.board2.has_ships();
    eosio_assert(!game_over,
                 "The game is already in an end state. You must decommit");
    data.state = P2_ATTACKED;
  }

  // player1 attacks are marked on board2 and vice versa
  const auto &attacker_board = is_player1 ? data.board1 : data.board2;
  auto &attackee_board = is_player1 ? data.board2 : data.board1;
  attackee_board.attack(attacks, attacker_board);
}

void automaton::reveal(bool is_player1,
                       const std::vector<uint8_t> &attack_responses) {
  if (is_player1) {
    eosio_assert(data.state == P1_ATTACKED, "P1 must attack first");
    data.state = P1_REVEALED;
  } else {
    eosio_assert(data.state == P1_REVEALED, "P1 must reveal first");
    data.state = P2_REVEALED;
  }

  (is_player1 ? data.board1 : data.board2).reveal(attack_responses);
}

void automaton::decommit(bool is_player1,
                         const eosio::checksum256 &decommitment) {
  if (is_player1) {
    eosio_assert(data.state == P2_VERIFIED, "P2 must verify first");
    data.board1.decommit(decommitment);
    bool p1_has_ships = data.board1.has_ships();
    bool p2_has_ships = data.board2.has_ships();
    data.state = (!p1_has_ships && !p2_has_ships)
                     ? DRAW
                     : (p1_has_ships ? P1_WIN : P2_WIN);
  } else {
    eosio_assert(data.state == P2_REVEALED, "P2 must reveal first");
    bool game_over = !data.board1.has_ships() || !data.board2.has_ships();
    eosio_assert(game_over, "The game is not over yet");
    data.state = P2_VERIFIED;
    data.board2.decommit(decommitment);
  }
}