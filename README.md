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

## Tests

`make test` runs `tree-sitter test` in every `catala_<lang>` directory
that has a `test/corpus` directory.

## License

The files contained in this repository are released under the [Apache
license (version 2)](LICENSE.txt) unless another license is explicited
for a sub-directory.
