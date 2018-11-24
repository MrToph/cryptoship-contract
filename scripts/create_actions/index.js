const fs = require(`fs-extra`)
const path = require(`path`)
const childProcess = require(`child_process`)
const ejs = require(`ejs`)

const contractDir = `./build`
const actionsDir = `./actions`
const actionTemplatesDir = `./scripts/create_actions`

const isNumberType = type => type.includes(`int`)

function createPayload(struct) {
    // {
    //     "name": "action",
    //     "base": "",
    //     "fields": [
    //         {
    //             "name": "user",
    //             "type": "name"
    //         },
    //     ]
    // },
    const obj = struct.fields.reduce(
        (acc, field) =>
            Object.assign(acc, {
                [field.name]: isNumberType(field.type)
                    ? Math.floor(Math.random() * 1000)
                    : `${field.name}`,
            }),
        {},
    )
    return JSON.stringify(obj, null, 4)
}

async function script() {
    try {
        const actionTemplate = fs.readFileSync(
            path.join(actionTemplatesDir, `action.template.js`),
            `utf8`,
        )
        const compile = ejs.compile(actionTemplate)

        const abi = JSON.parse(
            fs.readFileSync(path.join(contractDir, `cryptoship.abi`)),
            `utf8`,
        )

        await fs.emptyDir(actionsDir)
        abi.actions.forEach(action => {
            const data = {
                actionName: action.name,
                payload: createPayload(abi.structs.find(({ name }) => name === action.name)),
            }
            fs.writeFileSync(path.join(actionsDir, `${action.name}.js`), compile(data))
        })

        fs.copyFileSync(
            path.join(actionTemplatesDir, `transfer.js`),
            path.join(actionsDir, `transfer.js`),
        )

        childProcess.execSync(`eslint --fix --quiet ${actionsDir}`, {
            stdio: `inherit`,
        })

        console.log(`SUCCESS`)
    } catch (ex) {
        console.error(`Error while creating actions\n`, ex.stack)
    }
}

script()
