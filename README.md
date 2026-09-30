# tree-sitter-{en,fr,pl}-catala

Catala grammar for [tree-sitter](https://github.com/tree-sitter/tree-sitter).

This module defines grammars for
[catala](https://github.com/CatalaLang/catala) files as well as a
emacs-lisp script for indentation (requires an emacs version > 29).

## Tests

`make test` runs `tree-sitter test` in every `catala_<lang>` directory
that has a `test/corpus` directory.

## License

The files contained in this repository are released under the [Apache
license (version 2)](LICENSE.txt) unless another license is explicited
for a sub-directory.
