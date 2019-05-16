const initEnvironment = require(`eosiac`)
const { createSeed, createCommitment } = require(`./utils`)

const { api, sendTransaction, env } = initEnvironment(`dev`)

const accounts = Object.keys(env.accounts)
const CONTRACT_ACCOUNT = accounts[1]
const PLAYER1 = accounts[2]
const PLAYER2 = accounts[3]

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
    const p1ShipIndexes = [2, 3, 4]
    const p1Seed = createSeed(p1ShipIndexes)
    const p1Commitment = createCommitment(p1Seed)

    const p2ShipIndexes = [21, 20, 19]
    const p2Seed = createSeed(p2ShipIndexes)
    const p2Commitment = createCommitment(p2Seed)

    beforeEach(async () => {
        await sendTransaction({
            account: CONTRACT_ACCOUNT,
            authorization: [
                {
                    actor: CONTRACT_ACCOUNT,
                    permission: `active`,
                },
            ],
            name: `testreset`,
            data: { max_games: 0 },
        })
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
                    account: CONTRACT_ACCOUNT,
                    name: `create`,
                    authorization: [
                        {
                            actor: PLAYER1,
                            permission: `active`,
                        },
                    ],
                    data: {
                        player: PLAYER1,
                        nonce: 0,
                        quantity: `0.1000 EOS`,
                        commitment: p1Commitment,
                    },
                },
                {
                    account: `eosio.token`,
                    name: `transfer`,
                    authorization: [
                        {
                            actor: PLAYER1,
                            permission: `active`,
                        },
                    ],
                    data: {
                        from: PLAYER1,
                        to: CONTRACT_ACCOUNT,
                        quantity: `0.1000 EOS`,
                        memo: `create`,
                    },
                },
            ])
            // join
            result = await sendTransaction([
                {
                    account: `eosio.token`,
                    name: `transfer`,
                    authorization: [
                        {
                            actor: PLAYER2,
                            permission: `active`,
                        },
                    ],
                    data: {
                        from: PLAYER2,
                        to: CONTRACT_ACCOUNT,
                        quantity: `0.1000 EOS`,
                        memo: `${gameId}`,
                    },
                },
                {
                    account: CONTRACT_ACCOUNT,
                    name: `join`,
                    authorization: [
                        {
                            actor: PLAYER2,
                            permission: `active`,
                        },
                    ],
                    data: {
                        player: PLAYER2,
                        nonce: 0,
                        game_id: gameId,
                        commitment: p2Commitment,
                    },
                },
            ])

            // round 1
            result = await sendTransaction([
                {
                    account: CONTRACT_ACCOUNT,
                    name: `attack`,
                    authorization: [
                        {
                            actor: PLAYER2,
                            permission: `active`,
                        },
                    ],
                    data: {
                        player: PLAYER2,
                        game_id: gameId,
                        attacks: [0, 1, 2, 3],
                    },
                },
            ])
            result = await sendTransaction([
                {
                    account: CONTRACT_ACCOUNT,
                    name: `attack`,
                    authorization: [
                        {
                            actor: PLAYER1,
                            permission: `active`,
                        },
                    ],
                    data: {
                        player: PLAYER1,
                        game_id: gameId,
                        attacks: [21, 22, 23, 24],
                    },
                },
                {
                    account: CONTRACT_ACCOUNT,
                    name: `reveal`,
                    authorization: [
                        {
                            actor: PLAYER1,
                            permission: `active`,
                        },
                    ],
                    data: {
                        player: PLAYER1,
                        game_id: gameId,
                        attack_responses: [2, 2, 3, 4],
                    },
                },
            ])

            // round 2
            result = await sendTransaction([
                {
                    account: CONTRACT_ACCOUNT,
                    name: `reveal`,
                    authorization: [
                        {
                            actor: PLAYER2,
                            permission: `active`,
                        },
                    ],
                    data: {
                        player: PLAYER2,
                        game_id: gameId,
                        attack_responses: [3, 2, 2, 2],
                    },
                },
                {
                    account: CONTRACT_ACCOUNT,
                    name: `attack`,
                    authorization: [
                        {
                            actor: PLAYER2,
                            permission: `active`,
                        },
                    ],
                    data: {
                        player: PLAYER2,
                        game_id: gameId,
                        attacks: [4, 5],
                    },
                },
            ])
            result = await sendTransaction([
                {
                    account: CONTRACT_ACCOUNT,
                    name: `attack`,
                    authorization: [
                        {
                            actor: PLAYER1,
                            permission: `active`,
                        },
                    ],
                    data: {
                        player: PLAYER1,
                        game_id: gameId,
                        attacks: [20],
                    },
                },
                {
                    account: CONTRACT_ACCOUNT,
                    name: `reveal`,
                    authorization: [
                        {
                            actor: PLAYER1,
                            permission: `active`,
                        },
                    ],
                    data: {
                        player: PLAYER1,
                        game_id: gameId,
                        attack_responses: [5, 2],
                    },
                },
            ])

            // round 3
            result = await sendTransaction([
                {
                    account: CONTRACT_ACCOUNT,
                    name: `reveal`,
                    authorization: [
                        {
                            actor: PLAYER2,
                            permission: `active`,
                        },
                    ],
                    data: {
                        player: PLAYER2,
                        game_id: gameId,
                        attack_responses: [4],
                    },
                },
                {
                    account: CONTRACT_ACCOUNT,
                    name: `decommit`,
                    authorization: [
                        {
                            actor: PLAYER2,
                            permission: `active`,
                        },
                    ],
                    data: {
                        player: PLAYER2,
                        game_id: gameId,
                        decommitment: p2Seed,
                    },
                },
            ])
            result = await sendTransaction({
                account: CONTRACT_ACCOUNT,
                name: `decommit`,
                authorization: [
                    {
                        actor: PLAYER1,
                        permission: `active`,
                    },
                ],
                data: {
                    player: PLAYER1,
                    game_id: gameId,
                    decommitment: p1Seed,
                },
            })

            const game = await getLatestGame()
            // P2 should have won, i.e. state 9
            return expect(game.game_data.state).toBe(9)
        } catch (ex) {
            console.log(ex.message)
        }
    })
})
