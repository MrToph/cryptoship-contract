const { api } = require(`../config`)
const { sendTransaction, getErrorDetail } = require(`../utils`)
const { createSeed, createCommitment } = require(`./utils`)

const { CONTRACT_ACCOUNT } = process.env

// running three actions sometimes goes beyond default 5s timeout
jest.setTimeout(20000)

const getPrintOutput = transaction =>
    transaction.processed.action_traces
        .map(trace => `${trace.console}${trace.inline_traces.map(t => `\n\t${t.console}`)}`)
        .join(`\n`)

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
    const p1ShipIndexes = [2, 3, 4]
    const p1Seed = createSeed(p1ShipIndexes)
    const p1Commitment = createCommitment(p1Seed)

    const p2ShipIndexes = [21, 20, 19]
    const p2Seed = createSeed(p2ShipIndexes)
    const p2Commitment = createCommitment(p2Seed)

    beforeEach(async () => {
        await sendTransaction({ name: `testreset` })
    })

    afterEach(async () => {
        const game = await getLatestGame()
        console.log(JSON.stringify(game, null, 2))
    })

    test(`example game`, async () => {
        expect.assertions(1)
        try {
            const gameId = 0
            let result

            // create
            result = await sendTransaction([
                {
                    name: `create`,
                    actor: `test1`,
                    data: {
                        player: `test1`,
                        nonce: 0,
                        quantity: `0.1000 EOS`,
                        commitment: p1Commitment,
                    },
                },
                {
                    account: `eosio.token`,
                    name: `transfer`,
                    actor: `test1`,
                    data: {
                        from: `test1`,
                        to: CONTRACT_ACCOUNT,
                        quantity: `0.1000 EOS`,
                        memo: `create`,
                    },
                },
            ])
            result = await sendTransaction([
                {
                    account: `eosio.token`,
                    name: `transfer`,
                    actor: `test2`,
                    data: {
                        from: `test2`,
                        to: CONTRACT_ACCOUNT,
                        quantity: `0.1000 EOS`,
                        memo: `${gameId}`,
                    },
                },
                {
                    name: `join`,
                    actor: `test2`,
                    data: {
                        player: `test2`,
                        nonce: 0,
                        game_id: gameId,
                        commitment: p2Commitment,
                    },
                },
            ])

            // round 1
            result = await sendTransaction([
                {
                    name: `attack`,
                    actor: `test2`,
                    data: {
                        player: `test2`,
                        game_id: gameId,
                        attacks: [0, 1, 2, 3],
                    },
                },
            ])
            result = await sendTransaction([
                {
                    name: `attack`,
                    actor: `test1`,
                    data: {
                        player: `test1`,
                        game_id: gameId,
                        attacks: [21, 22, 23, 24],
                    },
                },
                {
                    name: `reveal`,
                    actor: `test1`,
                    data: {
                        player: `test1`,
                        game_id: gameId,
                        attack_responses: [2, 2, 3, 4],
                    },
                },
            ])

            // round 2
            result = await sendTransaction([
                {
                    name: `reveal`,
                    actor: `test2`,
                    data: {
                        player: `test2`,
                        game_id: gameId,
                        attack_responses: [3, 2, 2, 2],
                    },
                },
                {
                    name: `attack`,
                    actor: `test2`,
                    data: {
                        player: `test2`,
                        game_id: gameId,
                        attacks: [4, 5],
                    },
                },
            ])
            result = await sendTransaction([
                {
                    name: `attack`,
                    actor: `test1`,
                    data: {
                        player: `test1`,
                        game_id: gameId,
                        attacks: [20],
                    },
                },
                {
                    name: `reveal`,
                    actor: `test1`,
                    data: {
                        player: `test1`,
                        game_id: gameId,
                        attack_responses: [5, 2],
                    },
                },
            ])

            // round 3
            result = await sendTransaction([
                {
                    name: `reveal`,
                    actor: `test2`,
                    data: {
                        player: `test2`,
                        game_id: gameId,
                        attack_responses: [4],
                    },
                },
                {
                    name: `decommit`,
                    actor: `test2`,
                    data: {
                        player: `test2`,
                        game_id: gameId,
                        decommitment: p2Seed,
                    },
                },
            ])
            result = await sendTransaction({
                name: `decommit`,
                actor: `test1`,
                data: {
                    player: `test1`,
                    game_id: gameId,
                    decommitment: p1Seed,
                },
            })

            // console.log(getPrintOutput(result))
            const game = await getLatestGame()
            // P2 should have won, i.e. state 9
            expect(game.game_data.state).toBe(9)
        } catch (ex) {
            console.log(getErrorDetail(ex))
        }
    })
})
