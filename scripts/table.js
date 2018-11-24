const fs = require(`fs`)
const { api } = require(`../config`)
const { getDeployableFilesFromDir } = require(`../utils`)

const { CONTRACT_ACCOUNT } = process.env
const contractDir = `./build`

async function script() {
    const tableName = process.argv[2]
    const { abiPath } = getDeployableFilesFromDir(contractDir)
    const abi = JSON.parse(fs.readFileSync(abiPath, `utf8`))
    const validTableNames = abi.tables
        ? abi.tables.map(({ name }) => `"${name}"`).join(` `)
        : `No table names`
    if (!tableName) {
        console.log(
            `Please pass a valid table name as an argument to this script.\nExample: npm run table -- games\nValid table names: ${validTableNames}`,
        )
        return
    }

    // https://developers.eos.io/eosio-nodeos/reference#get_table_rows
    const result = await api.rpc.get_table_rows({
        json: true,
        code: CONTRACT_ACCOUNT,
        scope: process.argv[3] || CONTRACT_ACCOUNT,
        table: tableName,
        lower_bound: 0,
        upper_bound: -1,
        limit: 9999,
        index_position: 1,
    })
    console.dir(result.rows)
}

script()
