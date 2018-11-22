#include "../cryptoship.hpp"

// This action is invoked by deferred transactions
// Its purpose is to clean "advance" games that are expired
// for instance by setting expired running games to game over
// or deleting finished games after their expiry date
void cryptoship::cleanup() {

}
