"""
Pilotwings 64 string codec

Provides string encoding and decoding to translate between utf-8 strings
and PW64 encoded u16 strings. This module registers the codecs in import
and can be used with .encode("pw64") and .decode("pw64") methods.
"""

import codecs
import string
import struct
from typing import Tuple

_pw64CharMap = {
    0x00: '0', 0x01: '1', 0x02: '2', 0x03: '3', 0x04: '4', 0x05: '5', 0x06: '6', 0x07: '7',
    0x08: '8', 0x09: '9', 0x0A: 'A', 0x0B: 'B', 0x0C: 'C', 0x0D: 'D', 0x0E: 'E', 0x0F: 'F',
    0x10: 'G', 0x11: 'H', 0x12: 'I', 0x13: 'J', 0x14: 'K', 0x15: 'L', 0x16: 'M', 0x17: 'N',
    0x18: 'O', 0x19: 'P', 0x1A: 'Q', 0x1B: 'R', 0x1C: 'S', 0x1D: 'T', 0x1E: 'U', 0x1F: 'V',
    0x20: 'W', 0x21: 'X', 0x22: 'Y', 0x23: 'Z', 0x24: 'a', 0x25: 'b', 0x26: 'c', 0x27: 'd',
    0x28: 'e', 0x29: 'f', 0x2A: 'g', 0x2B: 'h', 0x2C: 'i', 0x2D: 'j', 0x2E: 'k', 0x2F: 'l',
    0x30: 'm', 0x31: 'n', 0x32: 'o', 0x33: 'p', 0x34: 'q', 0x35: 'r', 0x36: 's', 0x37: 't',
    0x38: 'u', 0x39: 'v', 0x3A: 'w', 0x3B: 'x', 0x3C: 'y', 0x3D: 'z', 0x3E: '-', 0x3F: '#',
    0x40: '<', 0x41: '>', 0x42: ' ', 0x43: '\"', 0x44: '(', 0x45: ')', 0x46: '*', 0x47: '&',
    0x48: ',', 0x49: '.', 0x4A: '/', 0x4B: '!', 0x4C: '?', 0x4D: '\'', 0x4E: '＞', 0x4F: ':',
    0xD4: '%d', 0xFE: '\n', 0xFD: '\t'
}

def search_function(encoding: str):
    if encoding == "pw64":
        return codecs.CodecInfo(pw64_encode, str_decode, None, None)
    else:
        return None

def pw64_encode(text: str, errors="strict") -> Tuple[bytes, int]:
    """
    encode UTF-8 string to PW64 u16 string
    
    Parameters
    ----------
    text: str
        string to encode

    Returns
    -------
    (bytes, int)
        bytes: big-endian encoded PW64 string representation
        int: number of characters consumed

    Supported characters
    --------------------
        - [0-9A-Za-z-#<> "()*&,./!?':\\n\\t]
        - Wide `>` arrow used in sound settings
        - Bold font, enclosed with {}
        - Replacement token `%d`
        - `%x####` for unknown tokens
    """
    encData = b''
    isBold = False
    inSpecial = False
    hexLeft = 0
    hexVal = 0
    for c in text:
        val = -1
        if inSpecial:
            inSpecial = False
            if c == 'd':
                val = 0xD4
            elif c == 'x':
                hexLeft = 4
                hexVal = 0
        elif hexLeft > 0:
            hexVal = (hexVal * 0x10) + int(c, 16)
            hexLeft -= 1
            val = hexVal if hexLeft == 0 else -1
        elif c == '{':
            isBold = True
        elif c == '}':
            isBold = False
        elif c == '%':
            inSpecial = True
        else:
            if c in string.digits:
                val = ord(c) - ord('0')
            elif c in string.ascii_uppercase:
                val = ord(c) - ord('A') + 0x0A
            elif c in string.ascii_lowercase:
                val = ord(c) - ord('a') + 0x24
            else:
                lut = {
                    '-': 0x3E, '#': 0x3F, '<': 0x40, '>': 0x41, ' ': 0x42, '\"': 0x43, '(': 0x44, ')': 0x45,
                    '*': 0x46, '&': 0x47, ',':  0x48, '.': 0x49, '/': 0x4A, '!': 0x4B, '?': 0x4C, '\'': 0x4D,
                    '＞': 0x4E, ':': 0x4F, '\n': 0xFE, '\t': 0xFD
                }
                assert c in lut, f"Unknown char {c}"
                if c in lut:
                    val = lut[c]
            if isBold and val >= 0 and val < 0x60:
                val += 0x60
        if val >= 0:
            encData += struct.pack(">H", val)
    encData += struct.pack(">H", 0xFF)
    return encData, len(text)

def str_decode(data: bytes, errors="strict") -> Tuple[str, int]:
    """
    decode from raw bytes to UTF-8 string representation

    Parameters
    ----------
    data: bytes
        big-endian encoded stream of u16 representation of the string

    Returns
    -------
    (str, int)
        str: UTF-8 encoded string
        int: number of bytes consumed

    Supported characters
    --------------------
        - [0-9A-Za-z-#<> "()*&,./!?':\\n\\t]
        - Wide `>` arrow used in sound settings
        - Bold font, enclosed with {}
        - Replacement token replaced with `%d`
        - Unknown sequences replaced with `%x####`
    """
    res = ""
    isBold = False
    consumed = 0
    for code, in struct.iter_unpack(">H", data):
        consumed += 2
        if code >= 0x60 and code < 0xB0:
            code -= 0x60
            if not isBold:
                res += "{"
            isBold = True
        elif isBold:
            res += "}"
            isBold = False
        if code in _pw64CharMap:
            res += _pw64CharMap[code]
        elif code == 0xFF: # terminator
            break
        else:
            res += f"%x{code:04x}"
    if isBold:
        res += "}"
    return res, consumed

# register PW64 string codec when imported
codecs.register(search_function)