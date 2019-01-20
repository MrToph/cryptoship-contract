const fs = require(`fs`)
const path = require(`path`)
const { RpcError } = require(`eosjs`)

const { api } = require(`../config.js`)

const { CONTRACT_ACCOUNT } = process.env

const createAction = ({
    account = CONTRACT_ACCOUNT,
    name,
    actor = CONTRACT_ACCOUNT,
    data = {},
}) => ({
    account,
    name,
    authorization: [
        {
            actor,
            permission: `active`,
        },
    ],
    data,
})

const sendTransaction = async args => {
    const actions = Array.isArray(args) ? args.map(createAction) : [createAction(args)]
    return api.transact(
        {
            actions,
        },
        {
            blocksBehind: 3,
            expireSeconds: 30,
        },
    )
}

function getErrorDetail(exception) {
    try {
        if (exception instanceof RpcError) return JSON.stringify(exception.json, null, 2)
        const { error } = exception.json
        let errorString = error.what
        if (error.details > 0) errorString += `: ${error.details.join(` | `)}`
        return errorString
    } catch (err) {
        return exception && exception.message
    }
}

function getDeployableFilesFromDir(dir) {
    const dirCont = fs.readdirSync(dir)
    const wasmFileName = dirCont.find(filePath => filePath.match(/.*\.(wasm)$/gi))
    const abiFileName = dirCont.find(filePath => filePath.match(/.*\.(abi)$/gi))
    if (!wasmFileName) throw new Error(`Cannot find a ".wasm file" in ${dir}`)
    if (!abiFileName) throw new Error(`Cannot find an ".abi file" in ${dir}`)
    return {
        wasmPath: path.join(dir, wasmFileName),
        abiPath: path.join(dir, abiFileName),
    }
}

module.exports = {
    sendTransaction,
    getErrorDetail,
    getDeployableFilesFromDir,
}
