const { sendTransaction, getErrorDetail } = require(`../utils`)

const { CONTRACT_ACCOUNT } = process.env

async function action() {
    try {
        const transaction = await sendTransaction({
            name: `create`,
            actor: `test1`,
            data: {
                player: `test1`,
                nonce: 767,
                quantity: `0.1000 EOS`,
                commitment: `7f8a03fe2ab222f6f8aa305c15b1e257b2636af7b9162ebca260600340433d3c`,
            },
        })
        console.log(`SUCCESS`)
        console.log(
            transaction.processed.action_traces
                .map(trace => `${trace.console}${trace.inline_traces.map(t => `\n\t${t.console}`)}`)
                .join(`\n`),
        )
    } catch (error) {
        console.error(`${getErrorDetail(error)}`)
    }
}

action()
