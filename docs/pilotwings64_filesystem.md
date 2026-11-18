# Pilotwings 64 Filesystem Data

The Pilotwings 64 ROM includes a custom filesystem that stores the game's
assets. This document enumerates the data types and their processing functions.
Functions and addresses are given in RAM for the North American game.

See also the ImHex pattern file `pilotwings64_imhex.hexpat` which can be used
to decode the PW64 filesystem data.


## ROM Memory Map Overview

| ROM Offset |  Length  | RAM Address           | Description
|------------|----------|-----------------------|-----------------
| 0x000000   | 0x000030 | -                     | ROM Header
| 0x000030   | 0x000FD0 | 0x1FC00000-0x1FC00FCF | IPL3 bootloader
| 0x001000   | 0x050E30 | 0x80200050-0x80250E7F | Kernel code/data
| 0x051E30   | 0x08C8F0 | 0x802CA900-0x803571EF | App code/data
| 0x0DE720   | 0x000E84 | -                     | Compressed filesystem index
| 0x0DF5B0   | 0x5395BC | -                     | ROM Filesystem


## File UVRM::TABL / Filesystem Index

The first file at 0x0DE720 uses tag `UVRM` and contains the filesystem index.
It contains only one `TABL` entry which is MIO0 compressed. Once decompressed,
this table enumerates a pair of ("TYPE", size) for each filesystem entry. Each
type is represented by a 4-byte string, referred to as `tag` in this document.
The `size` is big-endian u32 that denotes the size of the data in bytes.

| Offset | Type    | Description
|--------|---------|----------------
|  0x00  | char[4] | tag
|  0x04  | u32     | size (in bytes)

The files are listed in order, so the ROM's code uses the aggregate sizes to compute
the offset in the filesystem where the file should exist. The base of the filesystem
is 0x0DF5B0, so that is also the location of the first file. For further examples,
the first 3 files are listed below, along with the compute ROM offsets

| Idx | Tag  | Size | FS Offset | ROM Offset
|-----|------|------|-----------|-----------
|  0  | UVSY | 0048 |  0x000000 |  0x0DF5B0
|  1  | UVEN | 0778 |  0x000048 |  0x0DF5F8
|  2  | UVLT | 0018 |  0x0007C0 |  0x0DFD70


## Filesystem

The filesystem begins at 0x0DF5B0 and contains all the records as enumerated in
the Filesystem Index. Each entry begins with a `FORM` tag, the length of the data
that follows, and the file's tag which matches the tag in the Index.

| Offset | Type    | Description
|--------|---------|------------------------
|    0x0 | char[4] | 'FORM' tag
|    0x4 | u32     | length of file data
|    0x8 | char[4] | tag (matches index tag)
|    0xC |         | file-dependent data

All files may have any number of `PAD ` fields at the start of their file-dependent data.
These are always 4 bytes contain 0x0 and are skipped over in the code.

### Filesystem File Type Overview

There are two categories of files: engine and user. Engine files are used by the game's
engine and would likely be reusable formats (e.g. models, textures, fonts, audio).
User files are specific to the game's data (e.g. landing pads, balloons, rings).

|  Tag   | Functions           | Description
|--------|---------------------|--------------------------------------------
| `TABL` | func_802246a0       | Table of FORM data entries
| `3VUE` | userPath_8034A4F8   | User path data
| `ADAT` | textLoadBlock       | ASCII text for on-screen messages
| `PDAT` | demoAttLoadPos      | Position and controller data for demo recordings
| `UPWL` | levelLoadMapObjects | User Pilotwings Level (map)
| `UPWT` | taskLoadCommObj     | User Pilotwings Task (test)
| `UVAN` | uvJanimLoad         | Animation
| `UVBT` | uvParseTopUVBT      | Blit
| `UVCT` | uvParseTopUVCT      | Contour
| `UVEN` | uvParseTopUVEN      | Environment
| `UVFT` | uvParseTopUVFT      | Font
| `UVLT` | uvParseTopUVLT      | Lights (unpopulated)
| `UVLV` | uvParseTopUVLV      | Level: container for lights, models, animation
| `UVMD` | uvParseTopUVMD      | Model
| `UVRM` | uvMemInitBlockHdr   | ROM information
| `UVSQ` | uvParseTopUVSQ      | Sequence
| `UVSX` | func_80204518       | Sound Bank (Container for .CTL/.TBL)
| `UVSY` | uvLevelInit         | System
| `UVTI` | uvParseTopUVTI      | Texture image
| `UVTP` | uvParseTopUVTP      | Texture palette
| `UVTR` | uvParseTopUVTR      | Terra
| `UVTX` | uvParseTopUVTX      | Texture


### File UVSY / System

`UVSY` contains one `COMM` entry with version of the file system and counts
of the types of the other files used.

| Offset | Type | Description
|--------|------|-----------------
|  0x00  | f32  | TBD, set to 1.0
|  0x04  | u16  | UV filesystem version
|  0x06  | u16  | count of UVMD
|  0x08  | u16  | count of UVCT
|  0x0A  | u16  | count of UVTX
|  0x0C  | u16  | count of UVEN
|  0x0E  | u16  | count of UVLT
|  0x10  | u16  | count of UVTR
|  0x12  | u16  | count of UVSQ
|  0x14  | u16  | count of UVLV
|  0x16  | u16  | count of UVAN
|  0x18  | u16  | count of UVFT
|  0x1A  | u16  | count of UVBT
|  0x1C  | u16  | count of user files
|  0x1E  | u16  | count of UVSX
|  0x20  | u16  | count of UVTP
|  0x22  | u16  | TBD, set to 0
|  0x24  |      | **Total length**


### File 3VUE

The 3VUE file contains user path data stored as quaternions (QUAT) and translations (XLAT).
Additionally, there is common (COMM) field which stores meta information.

#### 3VUE::COMM

| Offset | Type | Description
|--------|------|------------
|  0x00  | s32  | TBD
|  0x04  | s32  | TBD
|  0x08  | s32  | TBD
|  0x0C  | s32  | TBD
|  0x10  | s32  | model ID
|  0x14  | s16  | TBD
|  0x16  | ??   | TBD
|  0x20  |      | **Total length**

#### 3VUE::QUAT

| Offset | Type  | Description
|--------|-------|------------
|  0x00  | Vec4F | quaternion
|  0x10  | s32   | TBD
|  0x14  |       | **Total length**

| Offset | Type    | Description
|--------|---------|------------
|  0x00  | char[4] | `QUAT`
|  0x04  | u32     | length of following (bytes)
|  0x08  | u32     | count of entries
|  0x0C  | s32     | TBD
|  0x10  | QUAT[]  | array of entries

#### 3VUE::XLAT

| Offset | Type  | Description
|--------|-------|------------
|  0x00  | Vec3F | translation
|  0x0C  | s32   | TBD
|  0x10  |       | **Total length**

| Offset | Type    | Description
|--------|---------|------------
|  0x00  | char[4] | `XLAT`
|  0x04  | u32     | length of following (bytes)
|  0x08  | s32     | count of entries
|  0x0C  | s32     | TBD
|  0x10  | XLAT[]  | array of entries


### File ADAT / ASCII data

The ADAT file contains ASCII and custom encoded strings. It begins with `SIZE`
tag followed by an array of `NAME` and `DATA` entires. `NAME` entries contain
ASCII encoded strings used as short identifiers for the `DATA` entries which
contain the custom encoded text displayed to the user. Example:
```
NAME: ERASE_SEL2
DATA: 00 6A 00 95 00 88 00 42 00 9C 00 92 00 98 00 42 00 96 00 98 00 95 00 88 00 AC 00 FE 00 FF
DATA (ASCII): Are you sure?
```

#### ADAT::SIZE

The `SIZE` entry is ignored by the code, but appears to have the following fields.

| Offset | Type    | Description
|--------|---------|------------
|  0x00  | char[4] | `SIZE`
|  0x04  | u32     | length of following (bytes)
|  0x08  | s32     | count of `NAME` and `DATA` records (439)
|  0x0C  | u8[4]   | padding
| *0x10* |         | **Total length**

#### ADAT::NAME

| Offset | Type    | Description
|--------|---------|------------
|  0x00  | char[4] | `NAME`
|  0x04  | u32     | length of following (bytes)
|  0x08  | char[]  | null-terminated ASCII string

#### ADAT::DATA

The `DATA` field uses custom encoding with u16s that allow convenient lookup
when rendering fonts on the screen. It supports two font styles: normal and bold.
The bold letters are representing by adding 0x60 to the character.

| Chars     | Encoding
| ----------|------------
| `0` - `9` | 0x00 - 0x09
| `A` - `Z` | 0x0A - 0x23
| `a` - `z` | 0x24 - 0x3D
| `-`       | 0x3E
| `#`       | 0x3F
| `<`       | 0x40
| `>`       | 0x41
| ` `       | 0x42
| `\`       | 0x43
| `(`       | 0x44
| `)`       | 0x45
| `*`       | 0x46
| `&`       | 0x47
| `,`       | 0x48
| `.`       | 0x49
| `/`       | 0x4A
| `!`       | 0x4B
| `?`       | 0x4C
| `'`       | 0x4D
| `#`       | 0x4E
| `:`       | 0x4F
| `%`       | 0xD4
| `\t`      | 0xFD
| `\n`      | 0xFE
| `\0`      | 0xFF

| Offset | Type    | Description
|--------|---------|-------------------------------
|  0x00  | char[4] | `DATA`
|  0x04  | u32     | length of following (bytes)
|  0x08  | u16[]   | PW64 custom u16 encoded string


### File PDAT / Position Data

`PDAT` contains the position, angle, and controller button data used in
the demo recordings.

#### PDAT::PHDR / position header

`PHDR` contains the timing information for the position recording.

| Offset | Type    | Description
|--------|---------|------------
|  0x00  | char[4] | `PHDR`
|  0x04  | u32     | length (bytes)
|  0x08  | s32     | duration
|  0x0C  | s32     | count of `PPOS`
| *0x10* |         | **Total length**

#### PDAT::PPOS / position, angle

`PPOS` contains the position and rotation information.
Each data point contains the full `PPOS` header.

| Offset | Type    | Description
|--------|---------|------------
|  0x00  | char[4] | `PPOS`
|  0x04  | u32     | length (bytes)
|  0x08  | Vec3F   | position
|  0x14  | Vec3F   | rotation
| *0x20* |         | **Total length**

#### PDAT::RHDR / recording header

`RHDR` contains the meta data for the demo recording.
This includes test, class, and vehicle IDs.

| Offset | Type    | Description
|--------|---------|------------
|  0x00  | char[4] | `RHDR`
|  0x04  | u32     | length (bytes)
|  0x08  | s32     | test ID
|  0x0C  | s32     | TBD, unused but set in range [0-5]
|  0x10  | s32     | class ID
|  0x14  | s32     | vehicle ID
|  0x18  | s32     | TBD (always 0)
| *0x1C* |         | **Total length**

#### PDAT::RPKT / button recording packet

`RPKT` contains the controller recording packet data. Each data point contains
a full `RPKT` header and contains the time, analog stick, and button values.

| Offset | Type    | Description
|--------|---------|------------
|  0x00  | char[4] | `RPKT`
|  0x04  | u32     | length (bytes)
|  0x08  | f32     | timestamp
|  0x0C  | f32     | analog stick X
|  0x10  | f32     | analog stick Y
|  0x14  | u32     | button mask
| *0x18* |         | **Total length**


### File UPWL / PW64 Level

The `UPWL` is a user file that contains the Pilotwings 64 Level (map) data.
These data pertain to the map as a whole regardless of which vehicle or test
are selected. It includes things like bonus star and terrain switch points.

#### UPWL::LEVL / level meta data

`LEVL` contains counts of each of the types of entries that follow.

| Offset | Type    | Description
|--------|---------|------------
|  0x00  | char[4] | `LEVL`
|  0x04  | u32     | length (bytes)
|  0x08  | u8      | count of ESND
|  0x09  | u8      | count of WOBJ
|  0x0A  | u8      | count of LPAD
|  0x0B  | u8      | count of TOYS
|  0x0C  | u8      | count of TPTS
|  0x0D  | u8      | count of APTS
|  0x0E  | u8      | count of BNUS
|  0x0F  | pad     |
| *0x10* |         | **Total length**

#### UPWL::APTS / audio switch point

| Offset | Type    | Description
|--------|---------|------------
|  0x00  | char[4] | `APTS`
|  0x04  | u32     | length (bytes)
|  0x08  | APT[]   | array of entries

| Entry  | Type    | Description
|--------|---------|------------
|  0x00  | Vec3F   | switch point
|  0x0C  | f32     | TBD
|  0x10  | f32     | TBD
|  0x14  | f32     | TBD
|  0x18  | f32     | TBD
|  0x1C  | f32     | TBD
| *0x20* |         | **Total length**

#### UPWL::BNUS / bonus star

Bonus stars are the secret stars located in each of the four maps.
Each map only defines one bonus star, but the game can support up to two.

| Offset | Type    | Description
|--------|---------|------------
|  0x00  | char[4] | `BNUS`
|  0x04  | u32     | length (bytes)
|  0x08  | BNUS[]  | array of entries

| Entry  | Type    | Description
|--------|---------|-----------------
|  0x00  | Vec3F   | star position
|  0x0C  | f32     | TBD (always -0.0)
|  0x10  | f32     | TBD (always 0.0)
|  0x14  | f32     | TBD (always 0.0)
|  0x18  | s32     | TBD (0 or 1)
| *0x1C* |         | **Total length**

#### UPWL::ESND / environment sound

| Offset | Type    | Description
|--------|---------|------------
|  0x00  | char[4] | `ESND`
|  0x04  | u32     | length (bytes)
|  0x08  | ESND[]  | array of entries

| Entry  | Type    | Description
|--------|---------|-----------------
|  0x00  | Mtx4F   | TBD
|  0x40  | Vec3F   | TBD
|  0x4C  | Vec3F   | TBD
|  0x58  | s8      | TBD
|  0x59  | pad[3]  |
|  0x5C  | f32     | TBD
|  0x60  | f32     | TBD
|  0x64  | s32     | TBD
|  0x68  | f32     | TBD
|  0x6C  | f32     | TBD
|  0x70  | s8      | TBD
|  0x71  | pad[3]  |
|  0x74  | s32     | TBD
| *0x78* |         | **Total length**

#### UPWL::LPAD / landing pads

Landing pads are the *potential* landing pads used in the rocket belt and hang
glider tests. The Pilotwings Task file has a separate `UPWT::LPAD` entry
elaborating more details on the landing pad to use for a given test.

| Offset | Type    | Description
|--------|---------|------------
|  0x00  | char[4] | `LPAD`
|  0x04  | u32     | length (bytes)
|  0x08  | LPAD[]  | array of entries

| Entry  | Type   | Description
|--------|--------|--------------------------------------------
|  0x00  | Vec3F  | position
|  0x0C  | f32    | angle
|  0x10  | s32    | is used (0 in FS, updated by test)
|  0x14  | u8     | landing pad type (0, 1, 2). updated from `UWPT::LPAD`
|  0x15  | pad[3] |
| *0x18* |        | **Total length**

#### UPWL::TOYS / animated objects

Toys are the objects in maps that move about, such as the ferris while, speedboat,
and other planes and hang gliders.

| Offset | Type    | Description
|--------|---------|------------
|  0x00  | char[4] | `TOYS`
|  0x04  | u32     | length (bytes)
|  0x08  | Toy[]   | array of entries

| Entry  | Type   | Description
|--------|--------|------------
|  0x00  | Vec3F  | position
|  0x0C  | u8[4]  | TBD
| *0x10* |        | **Total length**

#### UPWL::TPTS / terrain switch points

| Offset | Type    | Description
|--------|---------|------------
|  0x00  | char[4] | `TPTS`
|  0x04  | u32     | length (bytes)
|  0x08  | TPTS[]  | array of entries

| Entry  | Type   | Description
|--------|--------|------------
|  0x00  | u8     | TBD
|  0x01  | pad[3] |
|  0x04  | Vec3F  | TBD
|  0x10  | f32    | TBD
|  0x14  | f32    | TBD
|  0x18  | s32    | TBD
|  0x1C  | f32    | TBD
|  0x20  | s32    | TBD
|  0x24  | f32    | TBD
|  0x28  | Vec3F  | TBD
| *0x34* |        | **Total length**

#### UPWL::WOBJ / wind objects

Wind objects are items placed in a level whose behavior interacts with the wind.
These are used for the windsocks and wind turbines.

| Offset | Type    | Description
|--------|---------|------------
|  0x00  | char[4] | `WOBJ`
|  0x04  | u32     | length (bytes)
|  0x08  | Wobj[]  | array of entries

| Entry  | Type   | Description
|--------|--------|------------
|  0x00  | Vec3F  | position
|  0x0C  | u8     | wobj type
|  0x0D  | pad[3] |
| *0x10* |        | **Total length**

| Type | Description
|------|-------------------
|   0  | windsock (jointed)
|   1  | wind turbine
|   2  | windsock (simple)


### File UPWT / PW64 Task

User file `UPWT` contains the Pilotwings 64 Task data that pertains to
individual test scenarios. It includes things like rings, balloons, and targets.
It additionally has a `COMM` sections holding counts and other meta information
and `NAME` and `INFO` sections containing the string identifier and ASCII
description of the test. Below is a table listing the data types used and a
brief description. These are parsed in `taskLoadCommObj`/0x80345CE4.

|  Tag   | Description
|--------|--------------------------------------------
| `BALS` | Balls/balloons
| `BTGT` | Ball target
| `CNTG` | Cannon target
| `FALC` | Falco and Falco domains
| `HOPD` | Jumble Hopper goal
| `HPAD` | Hover pad for rocket belt
| `LPAD` | Landing pads
| `LSTP` | Landing strips
| `LWIN` | Local winds
| `OBSV` | Unknown, not in filesystem, but parsed in code
| `SDFM` | Unknown, not in filesystem
| `TARG` | Target
| `THER` | Thermal
| `TPAD` | Takeoff pad


#### UPWT::COMM / task common data

`UPWT::COMM` is the common data structure mapping the task to the tuple
(class, vehicle, test) and contains the counts for all the objects included.

| Offset  | Type     | Description
|---------|----------|------------------------
|  0x000  | u8       | class (beg/A/B/pilot)
|  0x001  | u8       | vehicle
|  0x002  | u8       | test (or target) number
|  0x003  | u8       | TBD
|  0x004  | u8       | TBD
|  0x005  | pad[3]   |
|  0x008  | u8[4]    | TBD
|  0x00C  | u8[4]    | TBD
|  0x010  | s32      | TBD
|  0x014  | u8[0x18] | TBD
|  0x02C  | s32      | TBD
|  0x030  | u8[0x14] | TBD
|  0x044  | f32      | TBD
|  0x048  | Struct   | TBD
|  0x414  | u8[8]    | TBD
|  0x41C  | u8       | count of THER
|  0x41D  | u8       | count of LWIN
|  0x41E  | u8       | count of TPAD
|  0x41F  | u8       | count of LPAD
|  0x420  | u8       | count of LSTP
|  0x421  | u8       | count of RNGS
|  0x422  | u8       | count of BALS
|  0x423  | u8       | count of TARG
|  0x424  | u8       | count of HPAD
|  0x425  | u8       | count of BTGT
|  0x426  | u8       | count of PHTS
|  0x427  | u8       | count of FALC
|  0x428  | u8       | count of SDFM
|  0x429  | u8       | count of CNTG
|  0x42A  | u8       | count of HOPD
|  0x42B  | u8       | count of OBSV
|  0x42C  |          | **Total length**

#### UPWT::JPTX / string identifier

`JPTX` is a unique string identifier for the test. It usually follows the
format `<Class>_<Vehicle>_<Test#>`.
Example: Rocket Belt (RP), Beginner (E), test 1: `E_RP_1`

#### UPWT::NAME / short name

`NAME` is a short name for the test, not following a set pattern and not
always aligned with the other data.
Example: Rocket Belt (RP), Beginner (E), test 1: `RP Exp`

#### UPWT::INFO / developer ASCII description

`INFO` is an ASCII description for the test generated by the devs. It almost
never aligns with the actual test objectives.
Example: Rocket Belt (RP), Beginner (E), test 1:
```
pop the balloon land and land on the designated landing area
```

#### UPWT::BALS / balls and balloons

`BALS` are the balloons to pop or balls to push during rocket belt tests.

| Offset | Type   | Description
|--------|--------|--------------------------------
|  0x00  | Vec3F  | position vector
|  0x0C  | Vec3F  | likely rotation vector (unused)
|  0x18  | s32    | TBD
|  0x1C  | f32    | TBD
|  0x20  | u8     | type (0 = orange, 1 = green, 2 = blue)
|  0x21  | u8     | will split into 5 balls when popped
|  0x22  | pad[2] |
|  0x24  | f32    | TBD
|  0x28  | f32    | TBD
|  0x2C  | f32    | TBD
|  0x30  | f32    | scale
|  0x34  | s32    | points for popping
|  0x38  | s32    | copied, but unused
|  0x3C  | f32    | drag coefficient
|  0x40  | f32    | gravity
|  0x44  | u8     | TBD
|  0x45  | pad[3] |
|  0x48  | f32    | TBD
|  0x4C  | f32    | TBD
|  0x50  | f32    | TBD
|  0x54  | s32    | TBD
|  0x58  | s32    | TBD
|  0x5C  | f32    | TBD
|  0x60  | f32    | TBD
|  0x64  | f32    | gravity for the balls split
|  0x68  |        | **Total length**

#### UPWT::BTGT / ball target

`BTGT` is the cylindrical target for the two ball-pushing rocket belt tests.

| Offset | Type   | Description
|--------|--------|-----------------
|  0x00  | Vec3F  | position
|  0x0C  | s32    | terrain ID
|  0x10  | f32    | radius
|  0x14  | f32    | height
|  0x18  | u8     | unused, copied to local struct (always 0x01)
|  0x19  | u8     | points scored for a ball in this goal
|  0x1A  | pad[2] |
|  0x1C  |        | **Total length**

#### UPWT::CNTG / cannon targets

`CNTG` is the target used in the cannonball tests. Only one is listed in each
`UPWT` file since cannonball tests are unique having one file per each target.

| Offset | Type   | Description
|--------|--------|------------
|  0x00  | Vec3F  | position
|  0x0C  | f32    | TBD
|  0x10  | f32    | TBD
|  0x14  | f32    | TBD
|  0x18  | u8     | type
|  0x19  | pad[3] | TBD
|  0x1C  |        | **Total length**

#### UPWT::FALC / Falco domain

`FALC` includes the test data for Falco and the domain used for the two
gyrocopter tests where Falco must be defeated.

| Offset | Type   | Description
|--------|--------|------------
|  0x00  | Vec3F  | TBD
|  0x0C  | f32    | TBD
|  0x10  | u8     | TBD
|  0x11  | u8     | TBD
|  0x12  | u8     | TBD
|  0x13  | u8     | TBD
|  0x14  | pad[4] |
|  0x18  | f32    | TBD
|  0x1C  | f32    | TBD
|  0x20  | f32    | TBD
|  0x24  | f32    | TBD
|  0x28  | f32    | TBD
|  0x2C  | f32    | TBD
|  0x30  | f32    | TBD
|  0x34  | f32    | TBD
|  0x38  | f32    | TBD
|  0x3C  | f32    | TBD
|  0x40  | f32    | TBD
|  0x44  | f32    | TBD
|  0x48  | f32    | TBD
|  0x4C  | f32    | TBD
|  0x50  | f32    | TBD
|  0x54  | f32    | TBD
|  0x58  | f32    | TBD
|  0x5C  | f32    | TBD
|  0x60  | f32    | TBD
|  0x64  | f32    | TBD
|  0x68  | f32    | TBD
|  0x6C  | f32    | TBD
|  0x70  | f32    | TBD
|  0x74  | f32    | TBD
|  0x78  | f32    | TBD
|  0x7C  | f32    | TBD
|  0x80  | f32    | TBD
|  0x84  | f32    | TBD
|  0x88  | f32    | TBD
|  0x8C  | f32    | TBD
|  0x90  | f32    | TBD
|  0x94  | f32    | TBD
|  0x98  | f32    | TBD
|  0x9C  | f32    | TBD
|  0xA0  | f32    | TBD
|  0xA4  | f32    | TBD
|  0xA8  | f32    | TBD
|  0xAC  |        | **Total length**

#### UPWT::HOPD / jumble hopper destination

`HOPD` is the target area for the jumble hopper tests.

| Offset | Type   | Description
|--------|--------|----------------------------
|  0x00  | ??     | TBD
|  0x04  | Vec3F  | X position
|  0x10  | s32    | TBD (always 0x01)
|  0x14  | f32    | scale (either 40.0 or 30.0)
|  0x18  | f32    | height (always 50.0)
|  0x1C  | pad[4] |
|  0x20  |        | **Total length**

#### UPWT::HPAD / hover pad

`HPAD` are the hovering pads used in two of the rocket belt tests.

| Offset | Type   | Description
|--------|--------|--------------------------------------
|  0x00  | Vec3F  | position
|  0x0C  | Vec3F  | rotation (degrees)
|  0x18  | s32    | TBD (always 0)
|  0x1C  | u8     | type (0=large, 1=standard)
|  0x1D  | u8     | points if landed on (typically 5 or 3)
|  0x1E  | pad[2] |
|  0x20  | f32    | fuel added upon landing [0.0-1.0]
|  0x24  | u8     | number of hover pads to activate next
|  0x25  | pad[3] |
|  0x28  | s32[5] | array of HPADs to activate
|  0x3C  | u8     | active at start
|  0x3D  | pad[3] |
|  0x40  |        | **Total length**

#### UPWT::LPAD / test landing pad

`UPWT::LPAD` are the landing pads to be used for a test. This differs from the
list of `UPWL::LPAD` potential landings pads level (map) data. There will only
be one test landing pad, but there can be many potential level landing pads.

| Offset | Type    | Description
|--------|---------|------------
|  0x00  | char[4] | `LPAD`
|  0x04  | u32     | length (bytes)
|  0x08  | LPAD[]  | array of entries

| Entry  | Type    | Description
|--------|---------|-------------------------------
|  0x00  | Vec3F   | position
|  0x0C  | Vec3F   | unused, always (0, 0, 0)
|  0x18  | pad[4]  |
|  0x1C  | Vec3F   | unused, always (0, 0, 0)
|  0x28  | pad[4]  |
|  0x2C  | u8      | pad type, used in model lookup
|  0x2D  | pad[3]  |
|  0x30  |         | **Total length**

#### UPWT::LSTP / landing strip

`LSTP` are the landing strips used for takeoff and landing in the gyrocopter tests.

| Offset | Type   | Description
|--------|--------|-----------------------------
|  0x00  | Vec3F  | upper-left position
|  0x0C  | Vec3F  | lower-right position
|  0x18  | pad[4] |
|  0x1C  | u8     | valid landing strip for test
|  0x1D  | pad[3] |
|  0x20  | f32    | landing alignment requirement
|  0x24  |        | **Total length**

#### UPWT::LWIN / local winds

`LWIN` are the localized winds setup for the test. These impact flight and
render in the radar as a blue wind arrow.

| Offset | Type   | Description
|--------|--------|---------------------
|  0x00  | Vec3F  | TBD
|  0x0C  | Vec3F  | TBD
|  0x18  | Vec3F  | TBD
|  0x24  | Vec3F  | TBD
|  0x30  | Vec3F  | TBD
|  0x3C  | s32    | TBD
|  0x40  | Vec3F  | TBD
|  0x4C  | f32    | TBD
|  0x50  | u8     | TBD
|  0x51  | u8     | TBD
|  0x52  | pad[2] |
|  0x54  |        | **Total length**

#### UPWT::OBSV

`OBSV` does not exist in any of the filesystem tasks, but is parsed in the code.
The parsed data includes a position vector and a float used for length comparison.

| Offset | Type  | Description
|--------|-------|----------------------
|  0x00  | Vec3F | positon
|  0x0C  | f32   | Some radius or length
|  0x10  |       | **Total length**

#### UPWT::PHTS / photos

`PHTS` contains the data for the three photography hang glider tests.
There is one entry for each photo required for the test.

| Offset | Type  | Description
|--------|-------|----------------------
|  0x00  | s32   | TBD
|  0x04  | s32   | TBD
|  0x08  | f32   | likely angle in degrees
|  0x0C  | f32   | likely angle in degrees
|  0x10  | f32   | TBD
|  0x14  |       | **Total length**

#### UPWT::RNGS / rings

`RNGS` are the rings to fly through during hang glider, rocket belt, and gyrocopter tests.

| Offset | Type   | Description
|--------|--------|--------------------
|  0x00  | Vec3F  | position
|  0x0C  | Vec3F  | angle (degrees)
|  0x18  | s32    | TBD
|  0x1C  | u8     | unknown, copied but unused [0-1]
|  0x1D  | u8     | child ring count
|  0x1E  | pad[2] |
|  0x20  | s32[5] | array of child ring indexes
|  0x34  | u8     | timed child ring count
|  0x35  | pad[3] |
|  0x38  | s32[5] | array of timed child ring indexes
|  0x4C  | u8     | points used for some rings
|  0x4D  | u8     | untimed ring
|  0x4E  | pad[2] |
|  0x50  | f32    | timed ring duration
|  0x54  | u8     | ring size
|  0x55  | u8     | is active
|  0x56  | pad[2] |
|  0x58  | f32    | rotation rate 0
|  0x5C  | f32    | x,y,z translation
|  0x60  | char   | rotation axis 0 ('x', 'y', 'z', or 'n')
|  0x61  | pad[3] |
|  0x64  | f32    | rotation rate 1 (after timeout)
|  0x68  | f32    | rotation rate 1 (before timeout)
|  0x6C  | f32    | rotation rate 1
|  0x70  | char   | rotation axis 1 ('x', 'y', 'z', or 'n')
|  0x71  | u8     | ring type
|  0x72  | u8     | ring sub-type, used to determine model
|  0x73  | u8     | unknown. copied but unused
|  0x74  | u8[16] | string name, always '\0'
|  0x84  |        | **Total length**

#### UPWT::SDFM

`SDFM` is not present in the filesystem and is only copied in code so only size is known.

| Offset | Type   | Description
|--------|--------|------------
|  0x00  | ??     | TBD
|  0x4C  |        | **Total length**

#### UPWT::TARG / targets

| Offset | Type   | Description
|--------|--------|------------
|  0x00  | Vec3F  | position
|  0x0C  | Vec3F  | rotation
|  0x18  | u8     | type
|  0x19  | u8     | TBD
|  0x1A  | pad[2] | TBD
|  0x1C  | s32    | TBD
|  0x20  |        | **Total length**

#### UPWT::THER / thermal

`THER` are the thermals in the hang glider missions.

| Offset | Type  | Description
|--------|-------|------------
|  0x00  | Vec3F | position
|  0x0C  | f32   | scale
|  0x10  | f32   | height
|  0x14  | s32   | TBD
|  0x18  | f32   | TBD
|  0x1C  | f32   | TBD
|  0x20  | f32   | TBD
|  0x24  | f32   | TBD
|  0x28  |       | **Total length**

#### UPWT::TPAD / takeoff pad

`TPAD` are the takeoff pads use for all test types.

| Offset | Type   | Description
|--------|--------|------------
|  0x00  | Vec3F  | position
|  0x0C  | Vec3F  | angle
|  0x18  | pad[4] |
|  0x1C  | Vec3F  | TBD
|  0x28  | u8     | on ground
|  0x29  | pad[3] | TBD
|  0x2C  | f32    | TBD (typically 1.0)
|  0x30  |        | **Total length**


### File UVLV / level

`UVLV` contains the counts and IDs used for the engine files

| Type  | Length            | Description
|-------|-------------------|------------------
| u16   | 2                 | terraCount
| u16[] | 2x terraCount     | terra IDs
| u16   | 2                 | lightCount
| u16[] | 2x lightCount     | light IDs
| u16   | 2                 | envCount
| u16[] | 2x envCount       | environment IDs
| u16   | 2                 | modelCount
| u16[] | 2x modelCount     | model IDs
| u16   | 2                 | contourCount
| u16[] | 2x contourCount   | countour IDs
| u16   | 2                 | textureCount
| u16[] | 2x textureCount   | texture IDs
| u16   | 2                 | sequenceCount
| u16[] | 2x sequenceCount  | sequence IDs
| u16   | 2                 | animationCount
| u16[] | 2x animationCount | animation IDs
| u16   | 2                 | fontCount
| u16[] | 2x fontCount      | font IDs
| u16   | 2                 | blitCount
| u16[] | 2x blitCount      | blit IDs


### File UVAN / animation

`UVAN` is animation data stored in MIO0 compressed blocks of `PART` data.


### File UVBT / blit

`UVBT` is blit data stored in MIO0 compressed blocks of `COMM` data.


### File UVCT / contours

`UVCT` is contour data in MIO0 compressed blocks of `COMM` data.


### File UVEN / environment

`UVEN` is environment data stored in `COMM` blocks.

| Offset | Type    | Description
|--------|---------|------------
|  0x00  | u8      | count
|  0x01  | Model[] | models of length count
|        | Data    |

| Model  | Type  | Description
|--------|-------|------------
|  0x00  | u16   | model ID
|  0x02  | u8    | flag
|  0x03  |       | **Total length**

|  Data  | Type    | Description
|--------|---------|------------
|  0x00  | Color   | Screen RGBA32
|  0x04  | Color   | Fog RGBA32
|  0x08  | u8      | TBD
|  0x09  | u8      | TBD
|  0x0A  | u8      | TBD
|  0x0B  | u8      | TBD
|  0x0C  | pad[8]  |
|  0x14  | f32     | TBD
|  0x18  | f32     | TBD
|  0x1C  | u8      | TBD
|  0x1D  | pad[17] |
|  0x2E  | u8      | TBD
|  0x2F  | u8      | TBD
|  0x30  | u32     | pointer
|  0x34  | u8      | count
|  0x35  | pad[3]  |
|  0x38  | u32     | pointer
|  0x3C  |         | **Total length**


### File UVFT / font

`UVFT` files contain the font faces used in the game. These are comprised of
`STRG` field which defines the supported ASCII characters, `BITM` bitmap data,
and `IMAG` data. The latter two are MIO0 compressed


### File UVLT / light (unpopulated)

`UVLT` file is present, but only contains a `PAD ` entry. The code attempts to
parse out a single `COMM` entry that is 4 bytes long.

### File UVMD / models

TBD

### File UVSQ / texture animation sequences

| Offset | Type   | Description
|--------|--------|-----------------
|  0x00  | u8     | frame count
|  0x01  | UVSQ[] | frame table
|        | u8     | mode
|        | u8     | reverse
|        | f32    | frame rate

| Frame  | Type  | Description
|--------|-------|-----------------------
|  0x00  | u16   | texture ID
|  0x02  | f32   | frame time (always 1.0)
|  0x06  |       | **Total length**


### File UVSX / sound bank

Container for .CTL/.TBL sound bank data.


### File UVTI / texture image

`UVTI` contains texture image data, but doesn't appear in the filesystem.
The code that parses it looks for the following structure

| Entry  | Type  | Description
|--------|-------|------------
|  0x00  | u16   | size (bytes, max: 4096)
|  0x02  | u16   | TBD
|  0x04  | u32   | TBD
|  0x08  | u32   | TBD
|  0x0C  | u32   | TBD
|  0x10  | u32   | TBD
|  0x14  | u8[]  | image data of length size


### File UVTP / texture palette

`UVTP` are texture palettes with one `COMM` field that has the following
structure.

| Offset | Type   | Description
|--------|--------|-----------------
|  0x00  | u16    | count of entries
|  0x02  | UVTP[] | sequence entries

| Entry  | Type  | Description
|--------|-------|------------
|  0x00  | u16   | TBD
|  0x02  | u16   | TBD
|  0x04  |       | **Total length**


### File UVTR / terrain

`UVTR` contains the terrain data. There is a common header followed by an array
of terrain tiles. If the tile type is 0x00, there is no further data for that
tile and the code will zero init the structure. If tile type is non-zero, a
matrix and other information follows.

| Offset | Type   | Description
|--------|--------|------------------------------
|  0x00  | f32    | TBD
|  0x04  | f32    | TBD
|  0x08  | f32    | TBD
|  0x0C  | f32    | TBD
|  0x10  | f32    | TBD
|  0x14  | f32    | TBD
|  0x18  | u8     | length
|  0x19  | u8     | width
|  0x1A  | pad[2] |
|  0x1C  | f32    | TBD
|  0x20  | f32    | TBD
|  0x24  | f32    | TBD
|  0x28  | Tile[] | files of size length * width

| Offset | Type  | Description
|--------|-------|----------------------------
|  0x00  | u8    | tile type (if 0, last byte)
|  0x01  | Mtx4F | 4x4 matrix
|  0x41  | u8    | TBD
|  0x42  | u16   | contour index
|  0x44  |       | **Total length**


### File UVTX / texture

`UVTX` contains texture data in a `COMM` entry which is MIO0 compressed.

## Additional Resources:
 - [Pilotwings 64 noclip research](https://github.com/magcius/noclip.website/wiki/Pilotwings-64-Research-&-Development)
 - [noclip | PW64 parsing code](https://github.com/magcius/noclip.website/blob/main/src/Pilotwings64/Scenes.ts)
 - [TCRF | Pilotwings 64](https://tcrf.net/Pilotwings_64)
 - [MIO0 Compression](https://hack64.net/wiki/doku.php?id=super_mario_64:mio0)
