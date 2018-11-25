const crypto = require(`crypto`)

function decimalToHex(d, padding = 2) {
    let hex = Number(d).toString(16)

    while (hex.length < padding) {
        hex = `0${hex}`
    }

    return hex
}

function createSeed(shipIndexes) {
    const shipIndexesHex = shipIndexes.map(v => decimalToHex(v)).join(``)
    // pad it with random bytes so it fits into 256 bit checksum type
    const randomBytes = crypto.randomBytes(32 - shipIndexesHex.length / 2)
    return `${shipIndexesHex}${randomBytes.toString(`hex`)}`
}

function createCommitment(seedAsHex) {
    // sha256 hash p1's message and save it in commitment
    const hash = crypto.createHash(`sha256`)
    hash.update(Buffer.from(seedAsHex, `hex`))
    const commitment = hash.digest(`hex`)
    return commitment
}

module.exports = {
    createSeed,
    createCommitment,
}
