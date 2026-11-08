#!/bin/env python3

import json
import os
from pathlib import Path
import struct
import time
from pw64 import filesys, mio0

def print_stats(fileTimes: dict):
    print("+------+-----+-------+-------+-------+")
    print("| tag  | #   | json  | bin   | total |")
    print("+------+-----+-------+-------+-------+")
    totTime = 0
    totCount = 0
    for tag, vals in sorted(fileTimes.items()):
        parse, generate = 0, 0
        for t in vals["times"]:
            parse += t[1] - t[0]
            generate += t[2] - t[1]
        count = len(vals["times"])
        totCount += count
        sum = parse + generate
        totTime += sum
        print(f"| {tag} | {count:>3d} | {parse:.3f} | {generate:.3f} | {sum:.3f} |")
    print("+------+-----+-------+-------+-------+")
    print(f"|Total | {totCount:>3d} |       |       | {totTime:.3f} |")
    print("+------+-----+-------+-------+-------+")

def read_table(tablePath):
    return json.load(open(tablePath, "r"))

def generate_bins(table: dict, fileDir: Path, tableFile: str, filesysFile: str, dumpData: bool, printStats: bool):
    stats = {}
    # 1. output filesystem binary and accumulate sizes
    with open(filesysFile, "wb") as fsBin:
        offset = 0
        for form in table["contents"]:
            fileName = Path(form["file"])
            filePath = fileDir / fileName
            if form["file"].endswith(".raw"):
                formData = open(filePath, "rb").read()
            elif form["file"].endswith(".json"):
                className = f"FORM_{form['tag']}" if form["tag"][0].isdigit() else form["tag"]
                assert hasattr(filesys, className), f"Unknown tag '{form['tag']}"
                generator = getattr(filesys, className)
                a = time.perf_counter()
                fileDat = json.load(open(filePath, "r"))
                b = time.perf_counter()
                formData = bytes(generator.from_dict(fileDat))
                c = time.perf_counter()
                if dumpData:
                    dumpPath = fileDir / "dump" / fileName.with_suffix(".bin")
                    os.makedirs(dumpPath.parent, exist_ok=True)
                    with open(dumpPath, "wb") as dumpFile:
                        dumpFile.write(formData)
                if form["tag"] not in stats:
                    stats[form["tag"]] = {"times": [], "parse": 0, "generate": 0}
                stats[form["tag"]]["times"].append((a, b, c))
            else:
                formData = b''
            formLen = len(formData)
            if formLen != form["length"]:
                print(f"Warning: length mismatch {form['file']} {form['length']} != {formLen}")
            form["length"] = formLen
            offset += formLen
            fsBin.write(formData)

    # 2. generate filetable, compress, and output file
    tableData = b''.join([struct.pack(">4s L", f["tag"].encode(), f["length"]) for f in table["contents"]])
    tableLen = len(tableData)
    # until matching MIO0 compression is found, use workaround instead of crunch64
    # cTableData = crunch64.mio0.compress(tableData)
    cTableData = mio0.compress(tableData, table["mio0_matching_info"])
    cTableLen = (((len(cTableData) + 8) + 3) // 4) * 4 # round up to 4-byte boundary
    uvRm = b'UVRM'
    uvRm += struct.pack(">4s L L", b'PAD ', 4, 0) * table["pad_count"]
    uvRm += struct.pack(">4s L", "GZIP".encode(), cTableLen)
    uvRm += struct.pack(">4s L", "TABL".encode(), tableLen)
    uvRm += cTableData
    uvRmLen = ((len(uvRm) + 3) // 4) * 4 # round up to 4 byte boundary
    with open(tableFile, "wb") as tableBin:
        tableBin.write(struct.pack(">4s L", "FORM".encode(), uvRmLen))
        tableBin.write(uvRm)

    if printStats:
        print_stats(stats)

def main():
    import argparse
    parser = argparse.ArgumentParser(
        description="PW64 filesystem generator",
        epilog="Example: python3 pw64_filesys_gen.py --dir ./filesys --dump --stats"
    )
    parser.add_argument("--table", "-t", action="store", default="./bin/filesys/filetable.json", help="input table file")
    parser.add_argument("--dir", "-d", action="store", default="./bin/filesys", help="input base directory")
    parser.add_argument("--dump", action="store_true", help="dump individual file binaries")
    parser.add_argument("--stats", action="store_true", help="print timing stats after generating file system")
    args = parser.parse_args()
    table = read_table(args.table)
    generate_bins(table, Path(args.dir), "bin/filetable.bin", "bin/filesys.bin", args.dump, args.stats)

if __name__ == "__main__":
    main()
