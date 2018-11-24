const { sendTransaction, getErrorDetail } = require(`../utils`)

const { CONTRACT_ACCOUNT } = process.env

async function action() {
    try {
        await sendTransaction({
            account: `eosio.token`,
            name: `transfer`,
            actor: `test1`,
            data: {
                from: `test1`,
                to: CONTRACT_ACCOUNT,
                quantity: `2.0000 EOS`,
                memo: new Date().toISOString(),
            },
        })
        console.log(`SUCCESS`)
    } catch (error) {
        console.error(`${getErrorDetail(error)}`)
    }
}

action()
