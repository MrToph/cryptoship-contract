const initEnvironment = require(`eosiac`)

const { sendTransaction } = initEnvironment(`dev`, { verbose: true })

const EOSIO_TOKEN_ACCOUNT = `eosio.token`

async function issueEosToken() {
    try {
        await sendTransaction({
            account: EOSIO_TOKEN_ACCOUNT,
            name: `create`,
            actor: EOSIO_TOKEN_ACCOUNT,
            data: {
                issuer: `eosio`,
                maximum_supply: `1000000000.0000 SYS`,
            },
        })
    } catch (error) {
        console.error(`Could not issue token: SYS `, error.message)
    }
    try {
        await sendTransaction({
            account: EOSIO_TOKEN_ACCOUNT,
            name: `create`,
            actor: EOSIO_TOKEN_ACCOUNT,
            data: {
                issuer: `eosio`,
                maximum_supply: `1000000000.0000 EOS`,
            },
        })
    } catch (error) {
        console.error(`Could not issue token: EOS `, error.message)
    }
}

issueEosToken()
