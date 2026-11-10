[![Build](https://github.com/n64decomp/pw64/actions/workflows/regress-internal.yaml/badge.svg)](https://github.com/n64decomp/pw64/actions/workflows/regress-internal.yaml)
[![Discord](https://img.shields.io/discord/1465499203831730191?color=%237289DA&logo=discord&logoColor=%23FFFFFF)](https://discord.gg/bkrYXSjrKE)

# Pilotwings 64 Decomp

This repo contains a **matching decompilation** of Pilotwings 64, currently supporting the following releases: Japan (J), North America (U).

The purpose of this project is to produce a reconstruction of the high-level source code through reverse engineering.

> [!IMPORTANT]
> This repository does not contain any game assets, assembly code, or other copyrighted materials.
> You must provide your own legally-obtained copy of the game in order to extract the assets necessary to build this repository.

## Versions

| Region | SHA1 | Code | Data | Functions |
|--------|------|------|------|-----------|
| (U)    | `ec771aedf54ee1b214c25404fb4ec51cfd43191a` | [![Code](https://decomp.dev/n64decomp/pw64/us.svg?mode=shield&measure=code&label=)](https://decomp.dev/n64decomp/pw64) | [![Data](https://decomp.dev/n64decomp/pw64/us.svg?mode=shield&measure=data&label=)](https://decomp.dev/n64decomp/pw64) | [![Functions](https://decomp.dev/n64decomp/pw64/us.svg?mode=shield&measure=matched_functions&label=)](https://decomp.dev/n64decomp/pw64) |
| (J)    | `892a5472d9a6811b881d093020de81f72baeeec9` | [![Code](https://decomp.dev/n64decomp/pw64/jp.svg?mode=shield&measure=code&label=)](https://decomp.dev/n64decomp/pw64) | [![Data](https://decomp.dev/n64decomp/pw64/jp.svg?mode=shield&measure=data&label=)](https://decomp.dev/n64decomp/pw64) | [![Functions](https://decomp.dev/n64decomp/pw64/jp.svg?mode=shield&measure=matched_functions&label=)](https://decomp.dev/n64decomp/pw64) |
| (E)    | Unsupported | 0% | 0% | 0 |


# Quick Start
See the [Development](#development) section below for full details on different systems. For Debian/Ubuntu, the following steps can be taken to quickly get going:
```bash
$ git clone --recurse-submodules https://github.com/n64decomp/pw64.git
$ cd pw64
$ sudo xargs -a packages.txt apt-get install -y
$ python3 -m venv .venv
$ source ./.venv/bin/activate
$ python3 -m pip install -r requirements.txt
$ cp <path-to-ROM> ./baserom.us.z64
$ make dependencies
$ make init
```

# Development

## Git
This repository uses Git submodules. Make sure to either clone the repository with the `--recurse-submodules` flag or to run `git submodule update --init --recursive` after the initial clone.

When pulling updates, you can update all submodules with `git submodule update --recursive`.

> Note: If you intend on developing inside of the Windows Subsystem for Linux (WSL), consider placing the repository on the WSL Linux partition. Builds are considerably slower when done on the Windows partition through WSL.

## Prerequisites

### Windows

Install the Windows Subsystem for Linux (WSL) and a Linux distribution according to the [Installation Guide](https://learn.microsoft.com/en-us/windows/wsl/install).

The following Linux instructions assume Ubuntu or Debian as the distribution.

### Debian/Ubuntu Linux (also under WSL)
The following packages are required for the build process:
- binutils-mips-linux-gnu (or similar, e.g. mips64-elf-binutils)
- build-essential
- git
- python3
- python3-pip

The following packages are also recommended but not strictly required:
- clang
- clang-format
- clang-tidy
- python3-venv

### macOS
First ensure that the following tools are installed:
- [Xcode Command Line Tools](https://developer.apple.com/documentation/xcode/command-line-tools )
- [Homebrew](https://brew.sh/)

The following homebrew packages are required for the build process:
- coreutils
- make
- tehzz/n64-dev/mips64-elf-binutils

### Python3 packages / venv
This project has python3 dependencies defined in the repo's requirements.txt. These can be managed through a python virtual environment and pip or through uv.

Example setup with python3 venv/pip
```bash
$ python3 -m venv .venv
$ source ./.venv/bin/activate
$ python3 -m pip install -r requirements.txt
```

## Setup
1. Place an unmodified Pilotwings 64 ROM into the root of the repository as `baserom.us.z64` or `baserom.jp.z64` (sha1sum must match Versions table above)
2. Set up dependencies
    - `make dependencies`
3. Init project
    - `make init`

## Rebuilding
1. If any symbols or the splat config were changed, re-extract the ROM and rebuild the linker script:
    - `make extract`
2. Rebuild the ROM:
    - `make -j`

## Docker
If you prefer to develop inside of a Docker container instead of installing everything in your local environment, use the provided Dockerfile in the root of the repository.

Example usage:
```bash
# Create image
docker build -t pw64decomp --build-arg login=$USER --build-arg uid=$UID .

# Enter a bash prompt
docker run --rm -it -v $(pwd):/pw64 pw64decomp bash

# Run a one-off command
docker run --rm -it -v $(pwd):/pw64 pw64decomp pw64 build
```

# Contributing
Pull requests are welcome. You can help with decompilation, renaming, documentation, and tooling. PRs are subject to code formatting checks, so there is an additional dependency of `clang-format` (provided by clang). Run `make format` to format your changes prior to starting the PR.
