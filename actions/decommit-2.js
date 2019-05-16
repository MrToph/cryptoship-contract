const initEnvironment = require(`eosiac`)

const { sendTransaction, env } = initEnvironment(`dev`, { verbose: true })

const p1Seed = `000102d6fac6ad67fc757f33640b35910d2029f7accb40935075fb1bd2f681a4`
const accounts = Object.keys(env.accounts)
const gameId = `0`

async function action() {
    try {
        await sendTransaction({
            account: accounts[1],
            name: `decommit`,
            authorization: [
                {
                    actor: accounts[2],
                    permission: `active`,
                },
            ],
            data: {
                player: accounts[2],
                game_id: gameId,
                decommitment: p1Seed,
            },
        })
    } catch (error) {
        // ignore
    }
}

action()
