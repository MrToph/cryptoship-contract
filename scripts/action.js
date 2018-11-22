const fs = require(`fs`)
const process = require(`process`)
const childProcess = require(`child_process`)

async function script() {
    const actionName = process.argv[2]
    const actionScriptPath = `actions/${actionName}.js`
    if (!actionName || !fs.existsSync(actionScriptPath)) {
        console.log(
            `Please pass a valid action name as an argument to this script.\nExample: npm run action -- transfer`,
        )
        return
    }

    childProcess.execSync(`node ${actionScriptPath}`, {
        stdio: `inherit`,
    })
}

script()
