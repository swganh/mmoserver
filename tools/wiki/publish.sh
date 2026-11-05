#!/usr/bin/env bash
#
# Copies the wiki pages kept in docs/wiki into a clone of the GitHub wiki and
# commits them. The wiki is a separate git repository:
#
#   git clone https://github.com/swganh/mmoserver.wiki.git ../mmoserver.wiki
#   tools/wiki/publish.sh ../mmoserver.wiki
#   git -C ../mmoserver.wiki push
#
# Page names come from the file names (Linux-Quick-Start.md is the page
# "Linux Quick Start"); _Sidebar.md and _Footer.md appear on every page.
# Pages that exist only in the wiki (the archived ones) are left alone.

set -euo pipefail

[[ $# -eq 1 ]] || { echo "usage: $(basename "$0") <wiki clone>" >&2; exit 2; }
wiki=$1
src=$(cd "$(dirname "${BASH_SOURCE[0]}")/../../docs/wiki" && pwd)

git -C "$wiki" rev-parse --git-dir >/dev/null 2>&1 || { echo "error: $wiki is not a git clone" >&2; exit 1; }

# A clone whose checkout failed (Windows cannot create files with ':' in
# their names) has an empty index; committing from it would delete pages.
if ! git -C "$wiki" diff --quiet HEAD -- 2>/dev/null || [[ -n $(git -C "$wiki" diff --cached --name-only --diff-filter=D) ]]; then
    echo "error: $wiki has changes or missing files; use a fresh, complete clone" >&2
    exit 1
fi

# The old home page is Textile; two Home pages would conflict.
if [[ -e $wiki/Home.textile ]]; then
    git -C "$wiki" rm -q Home.textile
fi

cp "$src"/*.md "$wiki"/
git -C "$wiki" add -- '*.md'

if git -C "$wiki" diff --cached --quiet; then
    echo "The wiki is already up to date."
    exit 0
fi

rev=$(git -C "$src" rev-parse --short HEAD)
git -C "$wiki" commit -q -m "Update from docs/wiki at $rev"
git -C "$wiki" show --stat --format='%h %s' HEAD
echo
echo "Review the pages, then publish with: git -C $wiki push"
