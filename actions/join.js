const initEnvironment = require(`eosiac`)

const { sendTransaction, env } = initEnvironment(`dev`, { verbose: true })

const accounts = Object.keys(env.accounts)

const gameId = 0
// 21, 20, 19
const p2Seed = `1514131bd3b0e2a57f5d67f202e46f122b920eb526cd613d80b9e7511db1f280`
const p2Commitment = `f5b0e70b7e8f5c40225b7a36b3d14bfb025b0adc10da78a21820280eaa7d67e0`

async function action() {
    try {
        await sendTransaction({
            account: accounts[1],
            name: `join`,
            authorization: [
                {
                    actor: accounts[3],
                    permission: `active`,
                },
            ],
            data: {
                player: accounts[3],
                nonce: 0,
                game_id: gameId,
                commitment: p2Commitment,
            },
        })
    } catch (error) {
        // ignore
    }
}

action()
