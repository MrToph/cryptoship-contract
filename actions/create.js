const initEnvironment = require(`eosiac`)

const { sendTransaction, env } = initEnvironment(`jungle`, { verbose: true })

const accounts = Object.keys(env.accounts)
// 0,1,2
const p1Seed = `000102d6fac6ad67fc757f33640b35910d2029f7accb40935075fb1bd2f681a4`
const p1Commitment = `a628623592c122609cbe929a0ef90eb0c19942ea52af65ee9db799daeeecd74a`

async function action() {
    try {
        await sendTransaction({
            account: accounts[1],
            name: `create`,
            authorization: [
                {
                    actor: accounts[2],
                    permission: `active`,
                },
            ],
            data: {
                player: accounts[2],
                nonce: 767,
                quantity: `0.1000 EOS`,
                commitment: p1Commitment,
            },
        })
    } catch (error) {
        // ignore
    }
}

action()
