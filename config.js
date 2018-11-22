const { Api, JsonRpc, JsSignatureProvider } = require(`eosjs`)
const fetch = require(`node-fetch`) // node only; not needed in browsers
const { TextEncoder, TextDecoder } = require(`util`) // node only; native TextEncoder/Decoder
const ecc = require(`eosjs-ecc`)

const map = require(`lodash/map`)
const mapValues = require(`lodash/mapValues`)
const uniq = require(`lodash/uniq`)
const dotenv = require(`dotenv`)

function loadEnvironmentVars() {
    const SCRIPT_ENV = [`testnet`, `production`].includes(process.env.NODE_ENV)
        ? process.env.NODE_ENV
        : `development`
    console.log(`Loading environment "${SCRIPT_ENV}"`)

    dotenv.config({ path: `.${SCRIPT_ENV}.env` })
    return {
        ...process.env,
        SCRIPT_ENV,
    }
}

const {
    SCRIPT_ENV,
    CONTRACT_ACCOUNT,
    CONTRACT_PRIVATE_KEY,
    EOS_HTTP_ENDPOINT,
    EOS_CHAIN_ID,
    EOSIO_PRIVATE_KEY, // used in dev only
} = loadEnvironmentVars()

function getKeys() {
    // New deterministic key for the testi account.
    // Do NOT use this in production
    const contractPrivate = CONTRACT_PRIVATE_KEY || ecc.seedPrivate(CONTRACT_ACCOUNT)

    const keys = mapValues(
        {
            [CONTRACT_ACCOUNT]: contractPrivate,
            test1: ecc.seedPrivate(`test1`),
            test2: ecc.seedPrivate(`test1`),
        },
        privateKey => [privateKey, ecc.privateToPublic(privateKey)],
    )

    if (SCRIPT_ENV === `development`) console.log(keys)
    return keys
}

const keys = getKeys()

const signatureProvider = new JsSignatureProvider([
    EOSIO_PRIVATE_KEY,
    ...uniq(map(keys, ([privateKey]) => privateKey)),
])
const rpc = new JsonRpc(EOS_HTTP_ENDPOINT, { fetch })
const api = new Api({
    rpc,
    signatureProvider,
    chainId: EOS_CHAIN_ID,
    textDecoder: new TextDecoder(),
    textEncoder: new TextEncoder(),
})

module.exports = {
    api,
    keys,
}
