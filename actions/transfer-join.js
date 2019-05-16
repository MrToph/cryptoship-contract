const initEnvironment = require(`eosiac`)

const { sendTransaction, env } = initEnvironment(`dev`, { verbose: true })

const accounts = Object.keys(env.accounts)

async function action() {
    try {
        await sendTransaction({
            account: `eosio.token`,
            name: `transfer`,
            authorization: [
                {
                    actor: accounts[3],
                    permission: `active`,
                },
            ],
            data: {
                from: accounts[3],
                to: accounts[1],
                quantity: `0.1000 EOS`,
                memo: `0`,
            },
        })
    } catch (error) {
        // ignore
    }
}

action()
