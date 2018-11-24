cmake_minimum_required(VERSION 3.5)
project(cryptoship VERSION 1.0.0)

find_package(eosio.cdt)

# add contract
add_contract( cryptoship cryptoship cryptoship.cpp )

### add the path to where the ricardian contracts/clauses are found
target_compile_options( cryptoship.wasm PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR} )
