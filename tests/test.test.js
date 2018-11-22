const { sendTransaction } = require(`../utils`)

const { CONTRACT_ACCOUNT } = process.env

describe(`contract`, () => {
    beforeEach(async () => {
        const result = await sendTransaction({ name: `testreset` })
        console.dir(result)
    })

    test(`something`, async () => {
        expect(true).toBe(true)
    })
})
