#!/usr/bin/env bash
# Round-trip test: encrypt -> decrypt must reproduce the original bytes.
set -euo pipefail
cd "$(dirname "$0")/.."

BIN=./fileenc
WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT

echo "ENCRYPTION_KEY=42" > "$WORK/.env"
ENV="$WORK/.env"
LOG="$WORK/test.log"

mkdir -p "$WORK/data/sub/deep"
echo "hello world"            > "$WORK/data/a.txt"
head -c 200000 /dev/urandom   > "$WORK/data/sub/b.bin"
printf ''                     > "$WORK/data/empty.txt"
echo "nested"                 > "$WORK/data/sub/deep/c.txt"
cp -r "$WORK/data" "$WORK/orig"

echo "== single file =="
$BIN encrypt "$WORK/data/a.txt" -e "$ENV" -l "$LOG" >/dev/null
$BIN decrypt "$WORK/data/a.txt.enc" -e "$ENV" -l "$LOG" >/dev/null
cmp "$WORK/orig/a.txt" "$WORK/data/a.txt.dec"
! cmp -s "$WORK/orig/a.txt" "$WORK/data/a.txt.enc"
rm "$WORK"/data/a.txt.*

echo "== directory (parallel) =="
$BIN encrypt "$WORK/data" -j 4 -e "$ENV" -l "$LOG" >/dev/null
test "$(find "$WORK/data" -name '*.enc' | wc -l)" -eq 4
$BIN decrypt "$WORK/data" -j 4 -e "$ENV" -l "$LOG" >/dev/null
for f in a.txt sub/b.bin empty.txt sub/deep/c.txt; do
    cmp "$WORK/orig/$f" "$WORK/data/$f.dec"
done

echo "== error handling =="
if $BIN encrypt "$WORK/nope" -e "$ENV" -l "$LOG" >/dev/null 2>&1; then echo "expected failure"; exit 1; fi
echo "BAD" > "$WORK/bad.env"
if $BIN encrypt "$WORK/data" -e "$WORK/bad.env" -l "$LOG" >/dev/null 2>&1; then echo "expected failure"; exit 1; fi

echo "ALL TESTS PASSED"
