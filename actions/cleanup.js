const initEnvironment = require(`eosiac`)

const { sendTransaction, env } = initEnvironment(`dev`, { verbose: true })

const accounts = Object.keys(env.accounts)

async function action() {
    try {
        await sendTransaction({
            account: accounts[1],
            name: `cleanup`,
            authorization: [
                {
                    actor: accounts[2],
                    permission: `active`,
                },
            ],
            data: {},
        })
    } catch (error) {
        // ignore
    }
}

action()
