const { deployContract } = require(`../utils`)

async function deploy() {
    const { CONTRACT_ACCOUNT } = process.env
    const contractDir = `./build`
    deployContract({ account: CONTRACT_ACCOUNT, contractDir })
}

deploy()
