const initEnvironment = require(`eosiac`)

const { sendTransaction, env } = initEnvironment(`dev`, { verbose: true })

const accounts = Object.keys(env.accounts)

async function action() {
    try {
        await sendTransaction({
            account: accounts[1],
            authorization: [
                {
                    actor: accounts[1],
                    permission: `active`,
                },
            ],
            name: `testreset`,
            data: { max_games: 0 },
        })
    } catch (error) {
        // ignore
    }
}

action()
