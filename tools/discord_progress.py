#!/usr/bin/env python3

import argparse
import requests
import subprocess
from git import Repo

repo = Repo('.')

def main(args):
    progress = subprocess.run(
        [ "python3", "-m", "mapfile_parser", "objdiff_report", "--version", args.version ],
        capture_output=True, text=True, check=True
    )
    progress_msg = progress.stdout

    commit = repo.heads.main.commit
    commit_msg = commit.message.splitlines()[0]

    content = {
        "embeds": [{
            "title": f"{commit_msg}",
            "description": f"{progress_msg}",
            "url": f"https://github.com/n64decomp/pw64/commit/{commit.hexsha}",
            "color": commit.authored_date & 0xFFFFFF
        }]
    }
    if args.skip_post:
        print(f"content: {content}");
    else:
        requests.post(args.discord_url, json=content)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Publish objdiff_report to Discord server")
    parser.add_argument("--discord_url", help="Discord webhook URL", required=True)
    parser.add_argument("--version", help="Select ROM version", default="us")
    parser.add_argument("--skip_post", help="Skip the post step", action='store_true')
    args = parser.parse_args()
    main(args)

