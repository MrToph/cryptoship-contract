# cryptoship

A battleship like game on the EOS blockchain.

## State Machine

Edges are the possible contract actions available from each state.

![State Machine](/.README/ch04_state_machine.png?raw=true "State Machine")

# Template

Requires `eosio-cpp (>= v1.4.0)` to be installed from the [eosio.cdt](https://github.com/EOSIO/eosio.cdt) package to compile the smart contract.

## Compiling

This template uses `cmake` to build the contract. Run the following commands once to setup the process:

```
mkdir build
cd build
cmake ../contract
```

Now you can run `npm run compile` which will run `make` to create the `.wasm` and `.abi` in `/build`.

## Deployment

Fill out the missing private key in `.testnet.env`, `.production.env`.

There's a `npm run init` script that _sets up your contract account_ and test accounts by creating them and transferring them enough EOS + RAM/NET/CPU.

> This should only be run on your local network to create accounts!

To deploy to the network specified in `.<environment>.env`, run:

```
NODE_ENV=testnet npm run deploy
```


## Testing the smart contract

You can run the following scripts to **automatically create scripts for your actions** defined in the ABI file.

```
npm run create_actions
```

You can then invoke these scripts to push actions to your deployed smart contract **without using cleos**:

```
npm run action -- <actionName>
```

Inspecting the contract's table can be done by:

```
npm run table -- <tableName>
```