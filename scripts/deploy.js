const { deployContract } = require(`../utils`)

async function deploy() {
    const { CONTRACT_ACCOUNT } = process.env
    const contractDir = `./contract`
    deployContract({ account: CONTRACT_ACCOUNT, contractDir })
}

deploy()
