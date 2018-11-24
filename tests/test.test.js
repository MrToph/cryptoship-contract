const { api } = require(`../config`)
const { sendTransaction } = require(`../utils`)

const { CONTRACT_ACCOUNT } = process.env

// running three actions sometimes goes beyond default 5s timeout
jest.setTimeout(20000)

const getLatestGame = async () =>
    api.rpc
        .get_table_rows({
            json: true,
            code: CONTRACT_ACCOUNT,
            scope: CONTRACT_ACCOUNT,
            table: `games`,
            lower_bound: 0,
            upper_bound: -1,
            limit: 9999,
        })
        .then(result => result.rows.pop())

describe(`contract`, () => {
    beforeEach(async () => {
        await sendTransaction({ name: `testreset` })
    })

    test(`something`, async () => {
        let result = await sendTransaction({
            account: `eosio.token`,
            name: `transfer`,
            actor: `test1`,
            data: {
                from: `test1`,
                to: CONTRACT_ACCOUNT,
                quantity: `0.1000 EOS`,
                memo: `create`,
            },
        })

        const game = await getLatestGame()
        expect(game).toBeDefined()
        console.dir(game)

        result = await sendTransaction({
            account: `eosio.token`,
            name: `transfer`,
            actor: `test2`,
            data: {
                from: `test2`,
                to: CONTRACT_ACCOUNT,
                quantity: `0.1000 EOS`,
                memo: `${game.id}`,
            },
        })
        expect(true).toBe(true)
    })
})
