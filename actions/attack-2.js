const initEnvironment = require(`eosiac`)

const { sendTransaction, env } = initEnvironment(`dev`, { verbose: true })

const accounts = Object.keys(env.accounts)
const gameId = `0`

async function action() {
    try {
        await sendTransaction({
            account: accounts[1],
            name: `attack`,
            authorization: [
                {
                    actor: accounts[2],
                    permission: `active`,
                },
            ],
            data: {
                player: accounts[2],
                game_id: gameId,
                attacks: [21, 22, 23, 24],
            },
        })
    } catch (error) {
        // ignore
    }
}

action()
