const { api, keys } = require(`../config`)
const { sendTransaction } = require(`../utils`)
const { getErrorDetail } = require(`../utils`)

const { CONTRACT_ACCOUNT } = process.env

async function createAccount(name, publicKey) {
    try {
        await api.rpc.get_account(name)
        console.log(`"${name}" already exists: ${publicKey}`)
        // no error => account already exists
        return
    } catch (e) {
        // error => account does not exist yet
    }
    console.log(`Creating "${name}" ${publicKey} ...`)
    await sendTransaction([
        {
            account: `eosio`,
            name: `newaccount`,
            actor: `eosio`,
            data: {
                creator: `eosio`,
                name,
                owner: {
                    threshold: 1,
                    keys: [
                        {
                            key: publicKey,
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
                            key: publicKey,
                            weight: 1,
                        },
                    ],
                    accounts: [],
                    waits: [],
                },
            },
        },
        // not needed on local network
        // {
        //     account: `eosio`,
        //     name: `buyrambytes`,
        //     actor: `eosio`,
        //     data: {
        //         payer: `eosio`,
        //         receiver: name,
        //         bytes: 1024 * 1024,
        //     },
        // },
        // {
        //     account: `eosio`,
        //     name: `delegatebw`,
        //     actor: `eosio`,
        //     data: {
        //         from: `eosio`,
        //         receiver: name,
        //         stake_net_quantity: `10.0000 SYS`,
        //         stake_cpu_quantity: `10.0000 SYS`,
        //         transfer: false,
        //     },
        // },
    ])

    await sendTransaction({
        account: `eosio.token`,
        name: `issue`,
        actor: `eosio`,
        data: {
            to: name,
            // SYS is configured as core symbol for creating accounts etc.
            // use EOS here to pay for contract
            quantity: `1000000.0000 EOS`,
            memo: `Happy spending`,
        },
    })
    console.log(`Done.`)
}

async function updateAuth() {
    console.log(`Updating auth with eosio.code ...`)
    const publicKey = keys[CONTRACT_ACCOUNT][1]
    const auth = {
        threshold: 1,
        keys: [{ key: publicKey, weight: 1 }],
        accounts: [
            { permission: { actor: CONTRACT_ACCOUNT, permission: `eosio.code` }, weight: 1 },
        ],
        waits: [],
    }
    await sendTransaction({
        account: `eosio`,
        name: `updateauth`,
        actor: CONTRACT_ACCOUNT,
        data: {
            account: CONTRACT_ACCOUNT,
            permission: `active`,
            parent: `owner`,
            auth,
        },
    })
    console.log(`Done.`)
}

async function init() {
    const accountNames = Object.keys(keys)
    for (const accountName of accountNames) {
        const [, publicKey] = keys[accountName]
        try {
            // eslint-disable-next-line no-await-in-loop
            await createAccount(accountName, publicKey)
        } catch (error) {
            console.error(`Cannot create account ${accountName} "${getErrorDetail(error)}"`)
            console.error(typeof error !== `string` ? JSON.stringify(error) : error)
        }
    }

    try {
        await updateAuth()
    } catch (error) {
        console.error(getErrorDetail(error))
        console.error(typeof error !== `string` ? JSON.stringify(error) : error)
    }
}

init()
