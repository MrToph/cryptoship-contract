# cryptoship

A battleship like game on the EOS blockchain.

## State Machine

Edges are the possible contract actions available from each state.

![State Machine](/.README/ch04_state_machine.png?raw=true "State Machine")

# Development

Requires `eosio-cpp (>= v1.6.1)` to be installed from the [eosio.cdt](https://github.com/EOSIO/eosio.cdt) package to compile the smart contract.
Also needs `cmake` for compiling the smart contract. (`brew install cmake`)

## Compiling

This template uses `cmake` to build the contract. Run the following commands once to setup the process:

```
mkdir build
cd build
cmake ../contract
```

Now you can run `npm run compile` which will run `make` to create the `.wasm` and `.abi` in `/build`.

## Setup & Deployment

To manage the contract & test accounts this template makes use of [eosiac](https://github.com/MrToph/eosiac).
Everything is configured in `eosiac.yml` and applied when run with:

```bash
eosiac apply dev
```


## Testing the smart contract

### Automated Tests

Run the tests in `tests` directory with `npm test`.

### Manually

There's an `actions` folder allowing an easy way to execute transactions to your deployed smart contract on any environment **without using cleos**:

```
npm run action -- <actionName>
EOSIAC_ENV=jungle npm run action -- create
```

Inspecting the contract's table can be done by:

```
npm run table -- <tableName>
```