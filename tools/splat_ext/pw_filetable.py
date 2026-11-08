import json
import struct
from splat.util import options
from splat.segtypes.segment import Segment
from splat.segtypes.linker_entry import LinkerEntry
from tools.pw64 import filesys, mio0

class N64SegPw_filetable(Segment):
    def __init__(self, rom_start, rom_end, type, name, vram_start, args, yaml):
        super().__init__(
            rom_start, rom_end, type, name, vram_start, args=args, yaml=yaml
        )
        self.align = 0x10
        self.fs_path = "filesys"

    def parse_table(self, rom_bytes: bytes, idx: int) -> dict:
        formTag, formLen = struct.unpack(">4s L", rom_bytes[idx:idx+8])
        print(f"form 0x{idx:x} 0x{formLen:x}")
        assert formTag == b'FORM', f"Expected 'FORM', got {formTag}"
        idx += 8
        fsDataStart = (((idx + formLen) + 15) // 16) * 16
        rtag, = struct.unpack(">4s", rom_bytes[idx:idx+4])
        assert rtag == b'UVRM', f"Expected 'UVRM', got {rtag}"
        idx += 4
        table = {
            "tag": formTag.decode(),
            "length": formLen,
            "type": rtag.decode(),
            "pad_count": 0,
            "contents": []
        }
        cur = 0
        fsDataOffset = fsDataStart
        while cur < formLen:
            tag, length = struct.unpack(">4s L", rom_bytes[cur+idx:cur+idx+8])
            cur += 8
            data = rom_bytes[cur+idx:cur+idx+length]
            # compressed data, unwrap MIO0 compression first
            if tag == b'GZIP':
                tag, dlength = struct.unpack(">4s L", rom_bytes[cur+idx:cur+idx+8])
                cur += 8
                cdata = rom_bytes[cur+idx:cur+idx+length-8]
                table["mio0_matching_info"] = {}
                data = mio0.decompress(cdata, table["mio0_matching_info"])
                assert len(data) == dlength, f"Expected {len(data)} == {dlength}"
            if tag == b'PAD ':
                table["pad_count"] += 1
            elif tag == b'TABL':
                for entryTag, entryLen in struct.iter_unpack(">4s L", data):
                    table["contents"].append({
                        "tag": entryTag.decode(),
                        "offset": fsDataOffset,
                        "length": entryLen
                    })
                    fsDataOffset += entryLen
            cur += length
        return table

    def scan(self, rom_bytes):
        self.fs_table = self.parse_table(rom_bytes, self.rom_start)

    def split(self, rom_bytes):
        path = options.opts.asset_path / self.dir / self.fs_path
        path.mkdir(parents=True, exist_ok=True)
        # assign file path and name which align with pw_filesys
        for form in self.fs_table["contents"]:
            className = f"FORM_{form['tag']}" if form["tag"][0].isdigit() else form["tag"]
            ext = "json" if hasattr(filesys, className) else "raw"
            form["file"] = f"FORM_{form['tag']}_{form['offset']:06X}.{ext}"
        # emit top-level filetable json
        assert self.fs_table and path, f"Unexpected {self.fs_table} {path}"
        with open(path / f"{self.name}.json", "w", newline="\n") as fout:
            json.dump(self.fs_table, fout, indent=2)

    def get_linker_entries(self):
        return [
            LinkerEntry(
                self,
                [options.opts.asset_path / self.dir / f"{self.name}.bin"],
                options.opts.asset_path / self.dir / f"{self.name}",
                self.get_linker_section_order(),
                self.get_linker_section_linksection(),
                self.is_noload(),
            )
        ]
