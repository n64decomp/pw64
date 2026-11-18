import crunch64
import hashlib
import struct

# MIO0 compression doesn't yet match Paradigm's algo
# collect compressed window information to be used during rebuild

def decompress(mio0Data: bytes, mio0Info: dict | None = None) -> bytes:
    """wrapper for MIO0 decompress which collects compressed window information"""

    compBlocks = []
    magic = mio0Data[:4]
    assert magic == b'MIO0', f"Expected 'MIO0', got {magic}"
    dLen, cOff, uOff = struct.unpack_from(">LLL", mio0Data, 4)
    flags = mio0Data[0x10:cOff]
    flagIdx = 0
    outSize = 0
    while outSize < dLen:
        flagOff = flagIdx // 8
        flagBit = 1 << (7 - (flagIdx % 8))
        flagIdx += 1
        if flags[flagOff] & flagBit != 0:
            outSize += 1
        else: # compressed
            windowInfo, = struct.unpack_from(">H", mio0Data, cOff)
            cOff += 2
            winOff = (windowInfo & 0x0FFF) + 1
            winLen = (windowInfo >> 12) + 3
            compBlocks.append({"file_offset": outSize, "offset": winOff, "length": winLen})
            outSize += winLen
    data = crunch64.mio0.decompress(mio0Data)
    if mio0Info is not None:
        mio0Info["sha1sum"] = hashlib.sha1(data).hexdigest()
        mio0Info["comp_blocks"] = compBlocks
    return data

def compress(data: bytes, mio0Info: dict | None = None) -> bytes:
    """wrapper for matching compression MIO0 workaround"""

    def PUT_BIT(buf, bit, val):
        mask = 1 << (7 - (bit % 8))
        offset = bit // 8
        while len(buf) < offset + 1:
            buf.append(0)
        buf[offset] = (buf[offset] & ~(mask)) | (mask if val else 0)

    if not mio0Info or hashlib.sha1(data).hexdigest() != mio0Info["sha1sum"]:
        cData = crunch64.mio0.compress(data)
    else:
        uBuf = b''
        cBuf = b''
        flags = []
        offset = 0x0
        bitIdx = 0
        for m in mio0Info["comp_blocks"]:
            uLen = m["file_offset"] - offset
            uBuf += data[offset:offset + uLen]
            offset += uLen
            for i in range(uLen):
                PUT_BIT(flags, bitIdx, 1)
                bitIdx += 1
            windowInfo = ((m["length"] - 3) << 12) | (m["offset"] - 1)
            cBuf += struct.pack(">H", windowInfo)
            PUT_BIT(flags, bitIdx, 0)
            bitIdx += 1
            offset += m["length"]
        uLen = len(data) - offset
        uBuf += data[offset:offset + uLen]
        offset += uLen
        for i in range(uLen):
            PUT_BIT(flags, bitIdx, 1)
            bitIdx += 1

        bitLength = (bitIdx + 7) // 8
        compOffset = (((0x10 + bitLength) + 3) // 4) * 4
        uncompOffset = compOffset + len(cBuf)
        cData = b'MIO0' + struct.pack(">L", len(data)) + struct.pack(">L", compOffset) + struct.pack(">L", uncompOffset)
        cData += bytes(flags) + cBuf + uBuf
    return cData
