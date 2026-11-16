#!/usr/bin/env python3
"""Prepare a release of EBST_CAM.

Sets the version in version.h, escam/escam.pro and escam_setup/escam_setup.vdproj
(with a new ProductCode and PackageCode), turns "Unreleased" in CHANGELOG.md into the
new version section, creates the commit "chore: release version x.y.z" and the tag x.y.z.
Nothing is pushed: check the result, then push commit and tag together, which starts
the release workflow on GitHub.

Usage:
    python scripts/prepare_release.py 4.25.0
    python scripts/prepare_release.py 4.25.0 --dry-run      # only show what would change
    python scripts/prepare_release.py 9.9.10 --any-branch   # test on a branch other than master
"""

import argparse
import datetime
import re
import subprocess
import sys
import uuid
from pathlib import Path

VERSION_PATTERN = re.compile(r"^\d+\.\d+\.\d+$")
UNRELEASED_PATTERN = re.compile(r"^##\s+\[?unreleased\]?\s*$", re.IGNORECASE)
REPO = Path(__file__).resolve().parent.parent


def git(*args, check=True):
    """Run a git command in the repository and return its output."""
    result = subprocess.run(["git", *args], cwd=REPO, capture_output=True, text=True)
    if check and result.returncode != 0:
        sys.exit(f"git {' '.join(args)} failed:\n{result.stderr.strip()}")
    return result.stdout.strip()


class TextFile:
    """A text file that is written back with the same encoding (BOM) and line endings."""

    def __init__(self, relative_path):
        self.name = relative_path
        self.path = REPO / relative_path
        raw = self.path.read_bytes()
        self.encoding = "utf-8-sig" if raw.startswith(b"\xef\xbb\xbf") else "utf-8"
        text = raw.decode(self.encoding)
        self.newline = "\r\n" if "\r\n" in text else "\n"
        self.final_newline = text.endswith("\n")
        self.lines = text.splitlines()

    def replace_one(self, pattern, replacement):
        """Replace the single line matching pattern (regex with groups); exit if not exactly one match."""
        regex = re.compile(pattern)
        matches = [i for i, line in enumerate(self.lines) if regex.search(line)]
        if len(matches) != 1:
            sys.exit(f"Expected exactly one line matching {pattern!r} in {self.name}, found {len(matches)}")
        i = matches[0]
        self.lines[i] = regex.sub(replacement, self.lines[i])

    def save(self):
        text = self.newline.join(self.lines) + (self.newline if self.final_newline else "")
        self.path.write_bytes(text.encode(self.encoding))


def heading_level(line):
    """Number of leading '#' of a Markdown heading, 0 for other lines."""
    match = re.match(r"^(#+)\s", line)
    return len(match.group(1)) if match else 0


def drop_empty_headings(lines):
    """Remove blank lines and headings without entries below them (before the next heading of the same or higher level)."""
    lines = [line for line in lines if line.strip()]
    kept = []
    for i, line in enumerate(lines):
        level = heading_level(line)
        if level == 0:
            kept.append(line)
            continue
        for following in lines[i + 1:]:
            following_level = heading_level(following)
            if following_level == 0:
                kept.append(line)
                break
            if following_level <= level:
                break
    return kept


def update_changelog(lines, version, date):
    """Turn "Unreleased" into "## <version> - <date>" and add a new empty "Unreleased" above it."""
    if any(re.match(rf"^##\s+\[?{re.escape(version)}\]?(\s|$)", line) for line in lines):
        sys.exit(f"CHANGELOG.md already contains a section for {version}")

    start = next((i for i, line in enumerate(lines) if UNRELEASED_PATTERN.match(line)), None)
    if start is None:
        sys.exit('CHANGELOG.md has no "## Unreleased" section')
    end = next((i for i in range(start + 1, len(lines)) if heading_level(lines[i]) == 2), len(lines))

    unreleased = lines[start + 1:end]
    entries = drop_empty_headings(unreleased)
    if not any(heading_level(line) == 0 for line in entries):
        sys.exit('"Unreleased" in CHANGELOG.md has no entries, nothing to release')

    # the new "Unreleased" gets the same (empty) headings as before, followed by as many blank lines as before
    template = [line for line in unreleased if heading_level(line) > 0]
    blank_lines = 0
    for line in reversed(unreleased):
        if line.strip():
            break
        blank_lines += 1
    blank_lines = max(blank_lines, 1)
    new_section = [lines[start], *template, *[""] * blank_lines, f"## {version} - {date}", *entries, ""]
    return lines[:start] + new_section + lines[end:]


def current_version(version_h):
    numbers = []
    for part in ("MAJOR", "MINOR", "PATCH"):
        match = next((re.search(rf"#define VERSION_{part}_ESCAM\s+(\d+)", l) for l in version_h.lines
                      if re.search(rf"#define VERSION_{part}_ESCAM\s+\d+", l)), None)
        if not match:
            sys.exit(f"VERSION_{part}_ESCAM not found in version.h")
        numbers.append(int(match.group(1)))
    return tuple(numbers)


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("version", help="new version, e.g. 4.25.0")
    parser.add_argument("--dry-run", action="store_true", help="only show the changes, do not write, commit or tag")
    parser.add_argument("--any-branch", action="store_true", help="allow a branch other than master (for tests)")
    args = parser.parse_args()
    version = args.version

    # 1. Checks
    if not VERSION_PATTERN.match(version):
        sys.exit(f"Invalid version '{version}', expected x.y.z")
    branch = git("rev-parse", "--abbrev-ref", "HEAD")
    if branch != "master" and not args.any_branch:
        sys.exit(f"You are on '{branch}', releases are made from master (use --any-branch for tests)")
    if git("status", "--porcelain", "--untracked-files=no"):
        sys.exit("Working tree has uncommitted changes, commit or stash them first")

    upstream = git("rev-parse", "--abbrev-ref", "@{u}", check=False)
    if upstream:
        remote = upstream.split("/", 1)[0]
        git("fetch", remote, "--tags")
        behind = git("rev-list", "--count", f"HEAD..{upstream}")
        if behind != "0":
            sys.exit(f"'{branch}' is {behind} commit(s) behind {upstream}, pull first")
    if git("rev-parse", "-q", "--verify", f"refs/tags/{version}", check=False):
        sys.exit(f"Tag {version} already exists")

    version_h = TextFile("version.h")
    escam_pro = TextFile("escam/escam.pro")
    vdproj = TextFile("escam_setup/escam_setup.vdproj")
    changelog = TextFile("CHANGELOG.md")

    old = current_version(version_h)
    new = tuple(int(x) for x in version.split("."))
    if new <= old:
        sys.exit(f"New version {version} is not higher than the current version {'.'.join(map(str, old))}")

    # 2. Changes
    major, minor, patch = version.split(".")
    for part, number in (("MAJOR", major), ("MINOR", minor), ("PATCH", patch)):
        version_h.replace_one(rf"^(#define VERSION_{part}_ESCAM\s+)\d+", rf"\g<1>{number}")
    for part, number in (("MAJOR", major), ("MINOR", minor), ("BUILD", patch)):
        escam_pro.replace_one(rf"^(VERSION_{part}_QT\s*=\s*)\d+", rf"\g<1>{number}")

    product_code = "{" + str(uuid.uuid4()).upper() + "}"
    package_code = "{" + str(uuid.uuid4()).upper() + "}"
    guid = r"\{[0-9A-Fa-f-]{36}\}"
    vdproj.replace_one(r'^(\s*"ProductVersion" = "8:)[^"]*(")', rf"\g<1>{version}\g<2>")
    vdproj.replace_one(rf'^(\s*"ProductCode" = "8:){guid}(")', rf"\g<1>{product_code}\g<2>")
    vdproj.replace_one(rf'^(\s*"PackageCode" = "8:){guid}(")', rf"\g<1>{package_code}\g<2>")

    changelog.lines = update_changelog(changelog.lines, version, datetime.date.today().isoformat())

    if args.dry_run:
        print(f"version.h, escam.pro, escam_setup.vdproj: {'.'.join(map(str, old))} -> {version}")
        print(f"escam_setup.vdproj: ProductCode {product_code}, PackageCode {package_code}")
        start = next(i for i, line in enumerate(changelog.lines) if line.startswith(f"## {version} "))
        end = next((i for i in range(start + 1, len(changelog.lines)) if heading_level(changelog.lines[i]) == 2),
                   len(changelog.lines))
        print("CHANGELOG.md:")
        print("\n".join(changelog.lines[start:end]).rstrip())
        print("\nDry run, nothing changed.")
        return

    files = (version_h, escam_pro, vdproj, changelog)
    for file in files:
        file.save()

    # 3. Commit and tag (local only)
    git("add", *(file.name for file in files))
    git("commit", "-m", f"chore: release version {version}")
    git("tag", version)

    print(git("show", "--stat", "--format=%h %s", "HEAD"))
    print(f"\nCreated commit and tag {version}. Check them, then push both together to every remote:")
    for remote in git("remote").split():
        print(f"    git push {remote} {branch} {version}")
    print("To undo before pushing:")
    print(f"    git tag -d {version}")
    print("    git reset --hard HEAD~1")


if __name__ == "__main__":
    main()
