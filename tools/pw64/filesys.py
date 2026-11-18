#!/bin/env python3

import struct
from . import codec

class FORM_3VUE: # prefix with `FORM_` since class names cannot begin with a number
    """User path data stored as quaternions and translations"""

    _commStruct = struct.Struct(">5l h 10x")
    _3vueTags = {
        "QUAT": struct.Struct(">4f l 4x"),
        "XLAT": struct.Struct(">3f l"),
    }

    def __init__(self, tag=None, pad_count=0, comm=None, entries=None):
        self.tag = tag if tag is not None else "3VUE"
        self.pad_count = pad_count
        self.comm = [] if comm is None else comm
        self.entries = {} if entries is None else entries

    @classmethod
    def from_dict(cls, d: dict):
        """Construct 3VUE from dictionary"""
        return cls(d["tag"], d["pad_count"], d["comm"], d["entries"])

    @classmethod
    def from_bytes(cls, form: bytes):
        """Construct 3VUE from raw filesystem bytes"""
        ftag, flen, vtag = struct.unpack_from(">4s L 4s", form)
        assert ftag == b'FORM', f"Expected 'FORM', got ${ftag}"
        vtag = vtag.decode()
        assert vtag == "3VUE", f"Expected '3VUE', got ${vtag}"
        idx = 0xC
        pad_count = 0
        comm = None
        entries = {}
        while idx < flen:
            tag, length = struct.unpack_from(">4s L", form, idx)
            idx += 8
            tag = tag.decode()
            assert tag in ("PAD ", "COMM", "QUAT", "XLAT"), f"Unexpected tag '${tag}'"
            if tag == "PAD ":
                pad_count += 1
            elif tag == "COMM":
                comm = list(cls._commStruct.unpack_from(form, idx))
            elif tag in cls._3vueTags:
                parser = cls._3vueTags[tag]
                vIdx = idx
                count, tbd = struct.unpack_from(">2L", form, vIdx)
                vIdx += 8
                blockLen = parser.size * count
                block = [list(e) for e in parser.iter_unpack(form[vIdx:vIdx+blockLen])]
                vIdx += blockLen
                entries[tag] = block
            idx += length
        return cls(vtag, pad_count, comm, entries)

    def as_dict(self) -> dict:
        """Generate dictionary suitable for creating JSON representation"""
        return {
            "tag": self.tag,
            "pad_count": self.pad_count,
            "comm": self.comm,
            "entries": self.entries
        }

    def __bytes__(self) -> bytes:
        """Generate raw bytes suitable for regenerating filesystem data"""
        records = struct.pack(">4s", self.tag.encode())
        records += struct.pack(">4s L L", b'PAD ', 4, 0) * self.pad_count
        records += b'COMM' + struct.pack(">L", self._commStruct.size) + self._commStruct.pack(*self.comm)
        for tag, dat in self.entries.items():
            parser = self._3vueTags[tag]
            block = struct.pack(">LL", len(dat), 0) + b''.join([parser.pack(*e) for e in dat])
            block += b'\0' * ((8 - (len(block) % 8)) % 8)
            records += struct.pack(">4s L", tag.encode(), len(block)) + block
        return b'FORM' + struct.pack(">L", len(records)) + records


class ADAT:
    """
    ADAT stores string identifiers and data strings encoded in custom u16 chars.
    """

    def __init__(self, tag=None, pad_count=0, entries=None):
        self.tag = tag if tag is not None else self.__class__.__name__
        self.pad_count = pad_count
        self.entries = entries if entries is not None else {}

    @classmethod
    def from_dict(cls, d: dict):
        """Construct ADAT from dictionary"""
        return cls(d["tag"], d["pad_count"], d["entries"])

    @classmethod
    def from_bytes(cls, form: bytes):
        pad_count = 0
        entries = []
        ftag, flen, tag = struct.unpack_from(">4s L 4s", form)
        assert ftag == b'FORM', f"Expected 'FORM', got ${ftag}"
        assert tag == b'ADAT', f"Expected 'ADAT', got ${tag}"
        idx = 0xC
        aSize = 0
        aName = ""
        while idx < flen:
            atag, alen = struct.unpack_from(">4s L", form, idx)
            idx += 8
            if atag == b'PAD ':
                pad_count += 1
            elif atag == b'SIZE':
                aSize, = struct.unpack_from(">L", form, idx)
            elif atag == b'NAME':
                aName = form[idx:idx+alen].decode().rstrip('\x00')
            elif atag == b'DATA':
                aDataRaw = form[idx:idx+alen]
                aData = aDataRaw.decode("pw64")
                entries.append({"NAME": aName, "DATA": aData})
            idx += alen
        assert len(entries) == aSize, f"Mismatch length: {len(entries)} != {aSize}"
        return cls("ADAT", pad_count, entries)

    def as_dict(self) -> dict:
        """Generate dictionary suitable for creating JSON representation"""
        return {
            "tag": self.tag,
            "pad_count": self.pad_count,
            "entries": self.entries
        }

    def __bytes__(self) -> bytes:
        """Generate raw bytes suitable for regenerating filesystem data"""
        records = struct.pack(">4s", self.tag.encode())
        records += struct.pack(">4s L L", b'PAD ', 4, 0) * self.pad_count
        records += struct.pack(">4s L L L", b'SIZE', 8, len(self.entries), 0)
        for ent in self.entries:
            name = ent["NAME"].encode() + b'\0'
            name += b'\0' * ((8 - (len(name) % 8)) % 8)
            encData = ent["DATA"].encode("pw64")
            encData += b'\0' * ((8 - (len(encData) % 8)) % 8)
            records += struct.pack(">4s L", b'NAME', len(name)) + name
            records += struct.pack(">4s L", b'DATA', len(encData)) + encData
        return struct.pack(">4s L", b'FORM', len(records)) + records


class PDAT:
    """Demo recording position, angle, and button data"""

    _pdatTags = {
        "PHDR": struct.Struct(">2l"),
        "PPOS": struct.Struct(">3f 3f"),
        "RHDR": struct.Struct(">5l"),
        "RPKT": struct.Struct(">f 2f L"),
    }

    def __init__(self, tag=None, pad_count=0, entries=None):
        self.tag = tag if tag is not None else self.__class__.__name__
        self.pad_count = pad_count
        self.entries = [] if entries is None else entries

    @classmethod
    def from_dict(cls, d: dict):
        """Construct PDAT from dictionary"""
        return cls(d["tag"], d["pad_count"], d["entries"])

    @classmethod
    def from_bytes(cls, form: bytes):
        """Construct PDAT from raw filesystem bytes"""
        ftag, flen, ptag = struct.unpack_from(">4s L 4s", form)
        assert ftag == b'FORM', f"Expected 'FORM', got ${ftag}"
        ptag = ptag.decode()
        assert ptag == cls.__name__, f"Expected '{cls.__name__}', got ${ptag}"
        idx = 0xC
        pad_count = 0
        entries = []
        while idx < flen:
            tag, length = struct.unpack_from(">4s L", form, idx)
            idx += 8
            tag = tag.decode()
            assert tag == "PAD " or tag in cls._pdatTags, f"Unexpected tag '${tag}'"
            if tag == "PAD ":
                pad_count += 1
            elif tag in cls._pdatTags:
                parser = cls._pdatTags[tag]
                entries.append([tag] + list(parser.unpack_from(form, idx)))
            idx += length
        return cls(ptag, pad_count, entries)

    def as_dict(self) -> dict:
        """Generate dictionary suitable for creating JSON representation"""
        return {
            "tag": self.tag,
            "pad_count": self.pad_count,
            "entries": self.entries,
        }

    def __bytes__(self) -> bytes:
        """Generate raw bytes suitable for regenerating filesystem data"""
        records = struct.pack(">4s", self.tag.encode())
        records += struct.pack(">4s L L", b'PAD ', 4, 0) * self.pad_count
        for dat in self.entries:
            tag = dat[0]
            parser = self._pdatTags[tag]
            block = parser.pack(*dat[1:])
            block += b'\0' * ((8 - (len(block) % 8)) % 8)
            records += struct.pack(">4s L", tag.encode(), len(block)) + block
        return b'FORM' + struct.pack(">L", len(records)) + records


class SPTH:
    """
    SPath class which stores x,y,z and heading,pitch,roll data for paths

    each of these datasets are stored as separate arrays, with each entry
    containing a time and value. the values used at runtime are then interpolated
    between the array. this means each dataset need not be of the same length.
    """

    # order of tags is important
    _spathTags = ("SCPP", "SCPH", "SCPX", "SCPY", "SCPR", "SCPZ", "SCP#")
    _timeVal = struct.Struct(">2f")

    def __init__(self, tag=None, pad_count=0, entries=None):
        self.tag = tag if tag is not None else self.__class__.__name__
        self.pad_count = pad_count
        self.entries = entries if entries is not None else {}

    @classmethod
    def from_dict(cls, d: dict):
        """Construct SPTH from dictionary"""
        return cls(d["tag"], d["pad_count"], d["entries"])

    @classmethod
    def from_bytes(cls, form: bytes):
        """Construct SPTH from raw filesystem bytes"""
        ftag, flen, stag = struct.unpack_from(">4s L 4s", form)
        assert ftag == b'FORM', f"Expected 'FORM', got ${ftag}"
        stag = stag.decode()
        assert stag == cls.__name__, f"Expected '{cls.__name__}', got ${stag}"
        idx = 0xC
        pad_count = 0
        entries = {}
        while idx < flen:
            tag, length = struct.unpack_from(">4s L", form, idx)
            idx += 8
            tag = tag.decode()
            assert tag == "PAD " or tag in cls._spathTags, f"Unexpected tag '${tag}'"
            if tag == "PAD ":
                pad_count += 1
            elif tag in cls._spathTags:
                spIdx = idx
                count, = struct.unpack_from(">L", form, spIdx)
                spIdx += 4
                sp = [{"time": e[0], "val": e[1]} for e in cls._timeVal.iter_unpack(form[spIdx:spIdx+8*count])]
                spIdx += 8*count
                entries[tag] = sp
            idx += length
        return cls(stag, pad_count, entries)

    def as_dict(self) -> dict:
        """Generate dictionary suitable for creating JSON representation"""
        return {
            "tag": self.tag,
            "pad_count": self.pad_count,
            "entries": self.entries
        }

    def __bytes__(self) -> bytes:
        """Generate raw bytes suitable for regenerating filesystem data"""
        records = struct.pack(">4s", self.tag.encode())
        records += struct.pack(">4s L L", b'PAD ', 4, 0) * self.pad_count
        for tag in self._spathTags:
            scpCount = len(self.entries[tag])
            scpLength = 8 + 8 * scpCount
            records += struct.pack(">4s L L", tag.encode(), scpLength, scpCount)
            records += b''.join([self._timeVal.pack(e['time'], e['val']) for e in self.entries[tag]])
            records += b'\0' * ((8 - (len(records) % 8)) % 8)
        spth = b'FORM' + struct.pack(">L", len(records)) + records
        return spth

class UPWL:
    """PW64 Level (map) data"""

    # order of tags must match order of LEVL counts
    _upwlTags = {
        "ESND": struct.Struct(">16f 3f 3f B 3x 2f L 2f b 3x L"),
        "WOBJ": struct.Struct(">3f B 3x"),
        "LPAD": struct.Struct(">3f f L B 3x"),
        "TOYS": struct.Struct(">3f 4B"),
        "TPTS": struct.Struct(">B 3x 3f 2f L f L f 3f"),
        "APTS": struct.Struct(">3f 5f"),
        "BNUS": struct.Struct(">3f 3f L")
    }

    def __init__(self, tag=None, pad_count=0, entries=None):
        self.tag = tag if tag is not None else self.__class__.__name__
        self.pad_count = pad_count
        self.entries = {} if entries is None else entries

    @classmethod
    def from_dict(cls, d: dict):
        """Construct UPWL from dictionary"""
        return cls(d["tag"], d["pad_count"], d["entries"])

    @classmethod
    def from_bytes(cls, form: bytes):
        """Construct UPWL from raw filesystem bytes"""
        ftag, flen, utag = struct.unpack_from(">4s L 4s", form)
        assert ftag == b'FORM', f"Expected 'FORM', got ${ftag}"
        utag = utag.decode()
        assert utag == cls.__name__, f"Expected '{cls.__name__}', got ${utag}"
        idx = 0xC
        pad_count = 0
        entries = {}
        counts = [0] * 8
        while idx < flen:
            tag, length = struct.unpack_from(">4s L", form, idx)
            idx += 8
            tag = tag.decode()
            assert tag in ("PAD ", "LEVL") or tag in cls._upwlTags, f"Unexpected tag '${tag}'"
            if tag == "PAD ":
                pad_count += 1
            elif tag == "LEVL":
                counts = struct.unpack_from(">7Bx", form, idx)
            elif tag in cls._upwlTags:
                expectedCount = counts[list(cls._upwlTags.keys()).index(tag)]
                entryStruct = cls._upwlTags[tag]
                entries[tag] = [list(v) for v in entryStruct.iter_unpack(form[idx:idx+expectedCount*entryStruct.size])]
            idx += length
        return cls(utag, pad_count, entries)

    def as_dict(self) -> dict:
        """Generate dictionary suitable for creating JSON representation"""
        return {
            "tag": self.tag,
            "pad_count": self.pad_count,
            "entries": self.entries
        }

    def __bytes__(self) -> bytes:
        """Generate raw bytes suitable for regenerating filesystem data"""
        records = struct.pack(">4s", self.tag.encode())
        records += struct.pack(">4s L L", b'PAD ', 4, 0) * self.pad_count
        counts = [0 if t not in self.entries else len(self.entries[t]) for t in self._upwlTags]
        records += b'LEVL' + struct.pack(">L 7B x", 8, *counts)
        for tag, vals in self.entries.items():
            entry = b''
            if tag in self._upwlTags:
                entry = b''.join([self._upwlTags[tag].pack(*e) for e in vals])
            entry += b'\0' * ((8 - (len(entry) % 8)) % 8)
            records += struct.pack(">4s L", tag.encode(), len(entry)) + entry
        upwl = b'FORM' + struct.pack(">L", len(records)) + records
        return upwl


class UPWT:
    """PW64 Task (test) data"""

    # order of tags must match order of COMM counts
    _upwtTags = {
        "THER": struct.Struct(">3f 2f L 4f"),
        "LWIN": struct.Struct(">3f 3f 3f 3f 3f L 3f f 2B 2x"),
        "TPAD": struct.Struct(">3f 3f 4x 3f B 3x f"),
        "LPAD": struct.Struct(">3f 3f 4x 3f 4x B 3x"),
        "LSTP": struct.Struct(">3f 3f 4x B 3x f"),
        "RNGS": struct.Struct(">3f 3f L 2B 2x 5L B 3x 5L 2B 2x f 2B 2x 2f b 3x 3f b 3B 16b"),
        "BALS": struct.Struct(">3f 3f L f B B 2x 4f 2L 2f B 3x 3f 2L 3f"),
        "TARG": struct.Struct(">3f 3f 2B 2x L"),
        "HPAD": struct.Struct(">3f 3f L 2B 2x f B 3x 5L B 3x"),
        "BTGT": struct.Struct(">3f L 2f 2B 2x"),
        "PHTS": struct.Struct(">2L 3f"),
        "FALC": struct.Struct(">3f f 4B 4B 37f"),
        "SDFM": struct.Struct(">76B"), # not present in FS
        "CNTG": struct.Struct(">3f 3f B 3x"),
        "HOPD": struct.Struct(">4B 3f L 2f 4B"),
        "OBSV": struct.Struct(">3f f"),
    }
    _upwtStrTags = ("JPTX", "NAME", "INFO")
    _commHeader = struct.Struct(">5B 3x 4B f")
    _commObjUnk10 = struct.Struct(">3f 4f")
    _commObjUnk2C = struct.Struct(">3f 3f")
    _commUnk10 = struct.Struct(">3f 4f")
    _commUnk2C = struct.Struct(">3f 3f")
    _commUnk803599D0 = struct.Struct(">L 16f 4L")
    _commUnk80345C80Footer = struct.Struct(">24B 2L 4f")

    def __init__(self, tag=None, pad_count=0, info=None, comm=None, entries=None):
        self.tag = tag if tag is not None else self.__class__.__name__
        self.pad_count = pad_count
        self.info = {} if info is None else info
        self.comm = {} if comm is None else comm
        self.entries = {} if entries is None else entries

    @classmethod
    def from_dict(cls, d: dict):
        """Construct UPWT from dictionary"""
        return cls(d["tag"], d["pad_count"], d["info"], d["comm"], d["entries"])

    @classmethod
    def from_bytes(cls, form: bytes):
        """Construct UPWT from raw filesystem bytes"""
        ftag, flen, utag = struct.unpack_from(">4s L 4s", form)
        assert ftag == b'FORM', f"Expected 'FORM', got ${ftag}"
        utag = utag.decode()
        assert utag == cls.__name__, f"Expected '{cls.__name__}', got ${utag}"
        idx = 0xC
        pad_count = 0
        info = {}
        comm = {}
        entries = {}
        counts = [0] * 16
        while idx < flen:
            tag, length = struct.unpack_from(">4s L", form, idx)
            idx += 8
            tag = tag.decode()
            assert tag in ("PAD ", "COMM") or tag in cls._upwtStrTags or tag in cls._upwtTags, f"Unexpected tag '${tag}'"
            if tag == "PAD ":
                pad_count += 1
            elif tag in cls._upwtStrTags:
                info[tag] = form[idx:idx+length].decode().rstrip('\x00')
                pass
            elif tag == "COMM":
                cIdx = idx
                comm["header"] = list(cls._commHeader.unpack_from(form, cIdx))
                cIdx += cls._commHeader.size
                comm["unk10"] = list(cls._commObjUnk10.unpack_from(form, cIdx))
                cIdx += cls._commObjUnk10.size
                comm["unk2C"] = list(cls._commObjUnk2C.unpack_from(form, cIdx))
                cIdx += cls._commObjUnk2C.size
                comm["unk44"], = struct.unpack_from(">f", form, cIdx)
                cIdx += 4
                comm["unk48"] = {}
                unk0Size = cls._commUnk803599D0.size * 11
                comm["unk48"]["unk0"] = [list(u) for u in cls._commUnk803599D0.iter_unpack(form[cIdx:cIdx+unk0Size])]
                cIdx += unk0Size
                comm["unk48"]["footer"] = list(cls._commUnk80345C80Footer.unpack_from(form, cIdx))
                cIdx += cls._commUnk80345C80Footer.size
                comm["unk414"] = list(struct.unpack_from(">4B", form, cIdx))
                cIdx += 4
                comm["unk418"], = struct.unpack_from(">f", form, cIdx)
                cIdx += 4
                counts = struct.unpack_from(">16B", form, cIdx)
                cIdx += 16
                assert cIdx - idx == 0x42C
            elif tag in cls._upwtTags:
                expectedCount = counts[list(cls._upwtTags.keys()).index(tag)]
                entryStruct = cls._upwtTags[tag]
                end = idx + expectedCount * entryStruct.size
                entries[tag] = [list(v) for v in entryStruct.iter_unpack(form[idx:end])]
            idx += length
        return cls(utag, pad_count, info, comm, entries)

    def as_dict(self) -> dict:
        """Generate dictionary suitable for creating JSON representation"""
        return {
            "tag": self.tag,
            "pad_count": self.pad_count,
            "info": self.info,
            "comm": self.comm,
            "entries": self.entries
        }

    def __bytes__(self) -> bytes:
        """Generate raw bytes suitable for regenerating filesystem data"""
        task = struct.pack(">4s", self.tag.encode())
        task += struct.pack(">4s L L", b'PAD ', 4, 0) * self.pad_count
        # JPTX/NAME/INFO
        for t, s in self.info.items():
            paddedStr = s.encode() + b'\0'
            paddedStr += b'\0' * ((8 - (len(paddedStr) % 8)) % 8)
            task += struct.pack(">4s L", t.encode(), len(paddedStr)) + paddedStr
        # COMM
        comm = self._commHeader.pack(*self.comm["header"])
        comm += self._commObjUnk10.pack(*self.comm["unk10"])
        comm += self._commObjUnk2C.pack(*self.comm["unk2C"])
        comm += struct.pack(">f", self.comm["unk44"])
        comm += b''.join([self._commUnk803599D0.pack(*e) for e in self.comm["unk48"]["unk0"]])
        comm += self._commUnk80345C80Footer.pack(*self.comm["unk48"]["footer"])
        comm += struct.pack(">4B", *self.comm["unk414"])
        comm += struct.pack(">f", self.comm["unk418"])
        counts = [0 if t not in self.entries else len(self.entries[t]) for t in self._upwtTags]
        comm += struct.pack(">16B", *counts)
        comm += b'\0' * ((8 - (len(comm) % 8)) % 8)
        task += b'COMM' + struct.pack(">L", len(comm)) + comm
        # entry types
        for tag, vals in self.entries.items():
            entry = b''
            if tag in self._upwtTags:
                entry = b''.join([self._upwtTags[tag].pack(*e) for e in vals])
            entry += b'\0' * ((8 - (len(entry) % 8)) % 8)
            task += struct.pack(">4s L", tag.encode(), len(entry)) + entry
        upwt = b'FORM' + struct.pack(">L", len(task)) + task
        return upwt


class UV_COMM:
    """Boilerplate class for UV** that have one type of COMM entry"""

    def __init__(self, tag=None, pad_count=0, comm=None):
        self.tag = tag if tag is not None else self.__class__.__name__
        self.pad_count = pad_count
        self.comm = [] if comm is None else comm

    @classmethod
    def from_dict(cls, d: dict):
        """Construct from dictionary"""
        return cls(d["tag"], d["pad_count"], d["comm"])

    def as_dict(self) -> dict:
        """Generate dictionary suitable for creating JSON representation"""
        return {
            "tag": self.tag,
            "pad_count": self.pad_count,
            "comm": self.comm
        }


class UVEN(UV_COMM):
    """Environment"""

    _dataStruct = struct.Struct(">4B 4B 4B 8x 2f B 17x 2B L B 3x L")

    @classmethod
    def from_bytes(cls, form: bytes):
        """Construct from raw filesystem bytes"""
        ftag, flen, utag = struct.unpack_from(">4s L 4s", form)
        assert ftag == b'FORM', f"Expected 'FORM', got ${ftag}"
        utag = utag.decode()
        assert utag == cls.__name__, f"Expected '{cls.__name__}', got ${utag}"
        idx = 0xC
        pad_count = 0
        comm = []
        while idx < flen:
            tag, length = struct.unpack_from(">4s L", form, idx)
            idx += 8
            tag = tag.decode()
            assert tag in ("PAD ", "COMM"), f"Unexpected tag '${tag}'"
            if tag == "PAD ":
                pad_count += 1
            else:
                models = []
                commIdx = idx
                count, = struct.unpack_from(">B", form, commIdx)
                commIdx += 1
                if count > 0:
                    models = [list(m) for m in struct.iter_unpack(">HB", form[commIdx:commIdx+3*count])]
                commIdx += 3*count
                data = list(cls._dataStruct.unpack_from(form, commIdx))
                commIdx += cls._dataStruct.size
                comm.append({"models": models, "data": data})
            idx += length
        return cls(utag, pad_count, comm)

    def __bytes__(self) -> bytes:
        """Generate raw bytes suitable for regenerating filesystem data"""
        records = struct.pack(">4s", self.tag.encode())
        records += struct.pack(">4s L L", b'PAD ', 4, 0) * self.pad_count
        for c in self.comm:
            comm = struct.pack(">B", len(c["models"]))
            comm += b''.join([struct.pack(">HB", m[0], m[1]) for m in c["models"]])
            comm += self._dataStruct.pack(*c["data"])
            comm += b'\0' * ((8 - (len(comm) % 8)) % 8)
            records += b'COMM' + struct.pack(">L", len(comm)) + comm
        return b'FORM' + struct.pack(">L", len(records)) + records


class UVLT(UV_COMM):
    """
    UVLT exists in the filesystem, but only contains 'PAD ' fields.
    The code will read 4 bytes from COMM if it existed, but it does not.
    """

    _ltStruct = struct.Struct(">4B")

    @classmethod
    def from_bytes(cls, form: bytes):
        """Construct from raw filesystem bytes"""
        ftag, flen, utag = struct.unpack_from(">4s L 4s", form)
        assert ftag == b'FORM', f"Expected 'FORM', got ${ftag}"
        utag = utag.decode()
        assert utag == cls.__name__, f"Expected '{cls.__name__}', got ${utag}"
        idx = 0xC
        pad_count = 0
        comm = []
        while idx < flen:
            tag, length = struct.unpack_from(">4s L", form, idx)
            idx += 8
            tag = tag.decode()
            assert tag in ("PAD ", "COMM"), f"Unexpected tag '${tag}'"
            if tag == "PAD ":
                pad_count += 1
            else:
                ltIdx = idx
                lt = list(cls._ltStruct.unpack_from(form, ltIdx))
                ltIdx += cls._ltStruct.size
                comm.append(lt)
            idx += length
        return cls(utag, pad_count, comm)

    def __bytes__(self) -> bytes:
        """Generate raw bytes suitable for regenerating filesystem data"""
        records = struct.pack(">4s", self.tag.encode())
        records += struct.pack(">4s L L", b'PAD ', 4, 0) * self.pad_count
        for c in self.comm:
            comm = self._ltStruct.pack(*c)
            comm += b'\0' * ((8 - (len(comm) % 8)) % 8)
            records += b'COMM' + struct.pack(">L", len(comm)) + comm
        return b'FORM' + struct.pack(">L", len(records)) + records


class UVLV(UV_COMM):
    """
    `UVLV` contains the counts and IDs used for the terrain, models, texture, animation for map data.
    """

    _levelFields = ("terra", "light", "environment", "model", "contour", "texture", "sequence", "animation", "font", "blit")

    @classmethod
    def from_bytes(cls, form: bytes):
        """Construct from raw filesystem bytes"""
        ftag, flen, utag = struct.unpack_from(">4s L 4s", form)
        assert ftag == b'FORM', f"Expected 'FORM', got ${ftag}"
        utag = utag.decode()
        assert utag == cls.__name__, f"Expected '{cls.__name__}', got ${utag}"
        idx = 0xC
        pad_count = 0
        comm = []
        while idx < flen:
            tag, length = struct.unpack_from(">4s L", form, idx)
            idx += 8
            tag = tag.decode()
            assert tag in ("PAD ", "COMM"), f"Unexpected tag '${tag}'"
            if tag == "PAD ":
                pad_count += 1
            else:
                lv = {}
                lvIdx = idx
                for field in cls._levelFields:
                    count, = struct.unpack_from(">H", form, lvIdx)
                    lvIdx += 2
                    lvLen = 2 * count
                    lv[field] = [i for i, in struct.iter_unpack(">H", form[lvIdx:lvIdx + lvLen])]
                    lvIdx += lvLen
                comm.append(lv)
            idx += length
        return cls(utag, pad_count, comm)

    def __bytes__(self) -> bytes:
        """Generate raw bytes suitable for regenerating filesystem data"""
        records = struct.pack(">4s", self.tag.encode())
        records += struct.pack(">4s L L", b'PAD ', 4, 0) * self.pad_count
        for c in self.comm:
            comm = b''
            for field in self._levelFields:
                ids = c[field]
                comm += struct.pack(">H", len(ids))
                comm += b''.join([struct.pack(">H", i) for i in ids])
            comm += b'\0' * ((8 - (len(comm) % 8)) % 8)
            records += b'COMM' + struct.pack(">L", len(comm)) + comm
        return b'FORM' + struct.pack(">L", len(records)) + records


class UVSQ(UV_COMM):
    """
    `UVSQ` contains animation sequences for dynamic textures.
    """

    _frameStruct = struct.Struct(">Hf")
    _uvsqStruct = struct.Struct(">B B f")

    @classmethod
    def from_bytes(cls, form: bytes):
        """Construct from raw filesystem bytes"""
        ftag, flen, utag = struct.unpack(">4s L 4s", form[:0xC])
        assert ftag == b'FORM', f"Expected 'FORM', got ${ftag}"
        utag = utag.decode()
        assert utag == cls.__name__, f"Expected '{cls.__name__}', got ${utag}"
        idx = 0xC
        pad_count = 0
        comm = []
        while idx < flen:
            tag, length = struct.unpack_from(">4s L", form, idx)
            idx += 8
            tag = tag.decode()
            assert tag in ("PAD ", "COMM"), f"Unexpected tag '${tag}'"
            if tag == "PAD ":
                pad_count += 1
            else:
                sqIdx = idx
                frameCount, = struct.unpack_from(">B", form, sqIdx)
                sqIdx += 1
                frameLen = 6 * frameCount
                frames = [{"textureId": t, "frameTime": f} for t, f in cls._frameStruct.iter_unpack(form[sqIdx:sqIdx + frameLen])]
                sqIdx += frameLen
                mode, reverse, framerate = cls._uvsqStruct.unpack_from(form, sqIdx)
                sq = {
                    "frames": frames,
                    "mode": mode,
                    "reverse": reverse,
                    "framerate": framerate
                }
                comm.append(sq)
            idx += length
        return cls(utag, pad_count, comm)

    def __bytes__(self) -> bytes:
        """Generate raw bytes suitable for regenerating filesystem data"""
        records = struct.pack(">4s", self.tag.encode())
        records += struct.pack(">4s L L", b'PAD ', 4, 0) * self.pad_count
        for c in self.comm:
            comm = struct.pack(">B", len(c["frames"]))
            comm += b''.join([self._frameStruct.pack(f["textureId"], f["frameTime"]) for f in c["frames"]])
            comm += self._uvsqStruct.pack(c["mode"], c["reverse"], c["framerate"])
            comm += b'\0' * ((8 - (len(comm) % 8)) % 8)
            records += b'COMM' + struct.pack(">L", len(comm)) + comm
        return b'FORM' + struct.pack(">L", len(records)) + records


class UVTP(UV_COMM):
    """
    `UVTP` contains texture palette information
    """

    _uvtpStruct = struct.Struct(">HH")

    @classmethod
    def from_bytes(cls, form: bytes):
        """Construct from raw filesystem bytes"""
        ftag, flen, utag = struct.unpack(">4s L 4s", form[:0xC])
        assert ftag == b'FORM', f"Expected 'FORM', got ${ftag}"
        utag = utag.decode()
        assert utag == cls.__name__, f"Expected '{cls.__name__}', got ${utag}"
        idx = 0xC
        pad_count = 0
        comm = []
        while idx < flen:
            tag, length = struct.unpack_from(">4s L", form, idx)
            idx += 8
            tag = tag.decode()
            assert tag in ("PAD ", "COMM"), f"Unexpected tag '${tag}'"
            if tag == "PAD ":
                pad_count += 1
            else:
                tpIdx = idx
                count, = struct.unpack_from(">H", form, tpIdx)
                tpIdx += 2
                tpLen = count * cls._uvtpStruct.size
                tp = [list(e) for e in cls._uvtpStruct.iter_unpack(form[tpIdx:tpIdx + tpLen])]
                tpIdx += tpLen
                comm.append(tp)
            idx += length
        return cls(utag, pad_count, comm)

    def __bytes__(self) -> bytes:
        """Generate raw bytes suitable for regenerating filesystem data"""
        records = struct.pack(">4s", self.tag.encode())
        records += struct.pack(">4s L L", b'PAD ', 4, 0) * self.pad_count
        for c in self.comm:
            comm = struct.pack(">H", len(c))
            comm += b''.join([self._uvtpStruct.pack(*i) for i in c])
            comm += b'\0' * ((8 - (len(comm) % 8)) % 8)
            records += b'COMM' + struct.pack(">L", len(comm)) + comm
        return b'FORM' + struct.pack(">L", len(records)) + records


class UVTR(UV_COMM):
    """
    `UVTR` contains terrain data.
    """

    _headerStruct = struct.Struct(">6f 2B 3f")
    _tileStruct = struct.Struct(">16f B H")

    @classmethod
    def from_bytes(cls, form: bytes):
        """Construct from raw filesystem bytes"""
        ftag, flen, utag = struct.unpack(">4s L 4s", form[:0xC])
        assert ftag == b'FORM', f"Expected 'FORM', got ${ftag}"
        utag = utag.decode()
        assert utag == cls.__name__, f"Expected '{cls.__name__}', got ${utag}"
        idx = 0xC
        pad_count = 0
        comm = []
        while idx < flen:
            tag, length = struct.unpack_from(">4s L", form, idx)
            idx += 8
            tag = tag.decode()
            assert tag in ("PAD ", "COMM"), f"Unexpected tag '${tag}'"
            if tag == "PAD ":
                pad_count += 1
            else:
                trIdx = idx
                header = list(cls._headerStruct.unpack_from(form, trIdx))
                trIdx += cls._headerStruct.size
                count = header[6] * header[7]
                tiles = []
                for _ in range(count):
                    tileType, = struct.unpack_from(">B", form, trIdx)
                    trIdx += 1
                    tile = [tileType]
                    if tileType != 0:
                        tile += list(cls._tileStruct.unpack_from(form, trIdx))
                        trIdx += cls._tileStruct.size
                    tiles.append(tile)
                tr = {
                    "header": header,
                    "tiles": tiles,
                }
                comm.append(tr)
            idx += length
        return cls(utag, pad_count, comm)

    def __bytes__(self) -> bytes:
        """Generate raw bytes suitable for regenerating filesystem data"""
        records = struct.pack(">4s", self.tag.encode())
        records += struct.pack(">4s L L", b'PAD ', 4, 0) * self.pad_count
        for c in self.comm:
            comm = self._headerStruct.pack(*c["header"])
            for t in c["tiles"]:
                comm += struct.pack(">B", t[0])
                if t[0] != 0:
                    comm += self._tileStruct.pack(*t[1:])
            comm += b'\0' * ((8 - (len(comm) % 8)) % 8)
            records += b'COMM' + struct.pack(">L", len(comm)) + comm
        return b'FORM' + struct.pack(">L", len(records)) + records
