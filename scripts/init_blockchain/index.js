const ecc = require(`eosjs-ecc`)
const { api } = require(`../../config`)
const { sendTransaction, getErrorDetail, deployContract } = require(`../../utils`)

const EOSIO_TOKEN_ACCOUNT = `eosio.token`

async function createTokenAccount() {
    const { EOSIO_PRIVATE_KEY } = process.env
    if (!EOSIO_PRIVATE_KEY)
        throw new Error(
            `process.env.EOSIO_PRIVATE_KEY is not set. Make sure to add it in ".development.env"`,
        )
    const EOSIO_PUBLIC_KEY = ecc.privateToPublic(EOSIO_PRIVATE_KEY)
    try {
        await api.rpc.get_account(EOSIO_TOKEN_ACCOUNT)
        console.log(`"${EOSIO_TOKEN_ACCOUNT}" already exists`)
        // no error => account already exists
        return
    } catch (e) {
        // error => account does not exist yet
    }

    await sendTransaction([
        {
            account: `eosio`,
            name: `newaccount`,
            actor: `eosio`,
            data: {
                creator: `eosio`,
                name: EOSIO_TOKEN_ACCOUNT,
                owner: {
                    threshold: 1,
                    keys: [
                        {
                            key: EOSIO_PUBLIC_KEY,
                            weight: 1,
                        },
                    ],
                    accounts: [],
                    waits: [],
                },
                active: {
                    threshold: 1,
                    keys: [
                        {
                            key: EOSIO_PUBLIC_KEY,
                            weight: 1,
                        },
                    ],
                    accounts: [],
                    waits: [],
                },
            },
        },
    ])
}

async function deployTokenContract() {
    const contractDir = `./scripts/init_blockchain/eosio.token`
    await deployContract({ account: EOSIO_TOKEN_ACCOUNT, contractDir })
}

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
        console.error(`Could not issue token: SYS `, getErrorDetail(error))
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
        console.error(`Could not issue token: EOS `, getErrorDetail(error))
    }
}

async function initToken() {
    await createTokenAccount()
    await deployTokenContract()
    await issueEosToken()
}

initToken()
