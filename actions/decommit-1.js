const initEnvironment = require(`eosiac`)

const { sendTransaction, env } = initEnvironment(`dev`, { verbose: true })

const p2Seed = `1514131bd3b0e2a57f5d67f202e46f122b920eb526cd613d80b9e7511db1f280`
const accounts = Object.keys(env.accounts)
const gameId = `0`

async function action() {
    try {
        await sendTransaction({
            account: accounts[1],
            name: `decommit`,
            authorization: [
                {
                    actor: accounts[3],
                    permission: `active`,
                },
            ],
            data: {
                player: accounts[3],
                game_id: gameId,
                decommitment: p2Seed,
            },
        })
    } catch (error) {
        // ignore
    }
}

action()
