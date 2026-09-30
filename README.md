# tree-sitter-{en,fr,pl}-catala

Catala grammar for [tree-sitter](https://github.com/tree-sitter/tree-sitter).

This module defines grammars for
[catala](https://github.com/CatalaLang/catala) files as well as a
emacs-lisp script for indentation (requires an emacs version > 29).

## Multi-word keywords

The grammars have an external scanner, `scanner.c`, which lexes the
multi-word keywords (`and then`, `list of`, `with pattern`, `égal à`,
`ou bien`, `plan każdy`...) as the compiler's lexer does:

- a multi-word keyword is a keyword only when each of its words is a
  whole word; directly followed by an identifier character, it is not that
  keyword, and its first word is lexed alone, as a keyword or an
  identifier: `a and then_x` is `a`, `and` and the identifier `then_x`,
  and `a ou bien_x` is `a`, `ou` and `bien_x`;
- its words are separated by any white space, newlines included, except
  where the compiler matches the keyword literally, with exactly one space:
  `in increasing order` and `in decreasing order` (English), `par ordre
  croissant` (French), `by increasing order` and `by decreasing order`
  (Polish);
- in English and French, `list of` and `optional of` (`liste de`,
  `optionnel de`) are single tokens, as in the compiler, so `list` and
  `optional` (`liste`, `optionnel`) alone are ordinary identifiers.

The French directive keywords `Usage de` and `en tant que` are regular
expressions that allow any horizontal white space, or none, between their
words, as the compiler's directive lexer does.

`make generate` writes `src/scanner.c` of each grammar from `scanner.c`
and the language's keyword table, `scanner_<lang>.h`, then checks with
`check_scanner.sh` that the grammar's externals and the table agree with
`scanner_tokens` and the `words(...)` tokens of `grammar.js`.

## ABI 14 and ABI 15 parsers

The committed parsers (`catala_<lang>`, `catala_code_<lang>`,
`catala_expr_<lang>`) are generated for tree-sitter ABI 14, which Emacs
requires. ABI 14 has no reserved words: a keyword is lexed as an
identifier wherever the parser expects an identifier and not that keyword.

The compiler lexes every keyword as a keyword, so the duration units
(`year`, `month`, `day`; `an`, `mois`, `jour`; `rok`, `miesiąc`, `dzień`)
are never identifiers: `input year content integer` is a syntax error.
`grammar.js` declares them as reserved words (`reserved`), which only ABI
15 parsers enforce. Generating for ABI 14 ignores them: the parse tables
are the same as without them, and `src/parser.c` differs only by its
unused `MAX_RESERVED_WORD_SET_SIZE`.

`make abi15` generates ABI 15 parsers of every grammar in `build/abi15`
(ignored by git), next to the committed ABI 14 ones; `make lib ABI=15
OUTDIR=build/abi15/catala_<lang> TREESITTER_CATALA_LANG=<lang>` builds one
of them as a native and a WebAssembly library.

## Tests

`make test` runs `tree-sitter test` in every `catala_<lang>` directory
that has a `test/corpus` directory, on the committed ABI 14 parser, and
skips the cases whose name ends with `(ABI 15)`. `make test-abi15` runs the
same corpus on an ABI 15 parser generated in `build/abi15`, and skips the
cases whose name ends with `(ABI 14)`: in `reserved.txt`, a duration unit
used as an identifier parses with ABI 14 and is an error with ABI 15. Both
rebuild the parser (`--rebuild`), because the tree-sitter CLI caches a
grammar's library by name only.

## License

The files contained in this repository are released under the [Apache
license (version 2)](LICENSE.txt) unless another license is explicited
for a sub-directory.
