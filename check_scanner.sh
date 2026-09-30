#!/bin/sh
# Usage: check_scanner.sh <lang> <grammar directory>
# Checks, after `tree-sitter generate`, that the grammar's externals are
# enum TokenType of scanner.c in the same order (a placeholder
# _<TOKEN>_NOT_SCANNED standing for <TOKEN>), and that the language's table
# (scanner_<lang>.h) has an entry for exactly the externals that the grammar
# leaves to the scanner (those that are not placeholders and have no rule of
# their own).
set -e
lang=$1
dir=$2
json=$dir/src/grammar.json

externals=$(node -e '
  const g = require(process.argv[1]);
  console.log(g.externals.map(e => e.name.replace(/^_(.*)_NOT_SCANNED$/, "$1"))
                         .join(" "))' "$PWD/$json")
scanned=$(node -e '
  const g = require(process.argv[1]);
  console.log(g.externals.filter(e => !(e.name in g.rules) &&
                                      !e.name.endsWith("_NOT_SCANNED"))
                         .map(e => e.name).sort().join(" "))' "$PWD/$json")
enum=$(sed -n '/^enum TokenType {/,/^};/s/^  \([A-Z_]*\),$/\1/p' scanner.c |
       tr '\n' ' ' | sed 's/ $//')
table=$(sed -n 's/^ *{\([A-Z_]*\), .*/\1/p' "scanner_$lang.h" |
        LC_ALL=C sort | tr '\n' ' ' | sed 's/ $//')
scanned=$(printf '%s\n' $scanned | LC_ALL=C sort | tr '\n' ' ' | sed 's/ $//')

if [ "$externals" != "$enum" ]; then
  echo "$dir: the externals of grammar.js differ from enum TokenType of scanner.c" >&2
  echo "  externals: $externals" >&2
  echo "  enum:      $enum" >&2
  exit 1
fi
if [ "$scanned" != "$table" ]; then
  echo "$dir: the words(...) tokens of grammar.js differ from scanner_$lang.h" >&2
  echo "  grammar.js:     $scanned" >&2
  echo "  scanner_$lang.h: $table" >&2
  exit 1
fi
