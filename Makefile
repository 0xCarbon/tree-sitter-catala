TREESITTER_CATALA_LANG ?= en
export TREESITTER_CATALA_LANG

TREESITTER_CATALA_VARIANT ?= catala
export TREESITTER_CATALA_VARIANT

SUBDIR = $(TREESITTER_CATALA_VARIANT)_$(TREESITTER_CATALA_LANG)

# The committed parsers, in $(SUBDIR), are generated for ABI 14, which Emacs
# requires (< 15). `make abi15` generates ABI 15 parsers, which also enforce
# the reserved words of grammar.js, in $(ABI15_DIR)/$(SUBDIR).
ABI = 14
ABI15_DIR = build/abi15
OUTDIR = $(SUBDIR)

LANGS = en fr pl

VARIANTS = catala catala_code catala_expr

COMBINATIONS = $(foreach V,$(VARIANTS),$(foreach L,$(LANGS),$V_$L))

LIB = $(OUTDIR)/libtree-sitter-$(TREESITTER_CATALA_VARIANT)_$(TREESITTER_CATALA_LANG).so

.PHONY: generate lib all test abi15 test-abi15 check-precedence check-reserved dist-clean ALWAYS

all: $(LANGS)

full: $(VARIANTS)

$(LANGS): Cargo.toml.in grammar.js ALWAYS
	$(MAKE) generate lib TREESITTER_CATALA_LANG=$@

$(VARIANTS): Cargo.toml.in grammar.js ALWAYS
	$(MAKE) $(LANGS) TREESITTER_CATALA_VARIANT=$@

lib: $(LIB)
$(LIB): generate
	@echo "Compiling $@ library"
	@cd $(OUTDIR) && tree-sitter build && tree-sitter build --wasm
	@cp -f $(OUTDIR)/parser.so $(@F)
	# @gcc -shared -fPIC -fno-exceptions -g -O2 -I $(TREESITTER_CATALA_LANG)/src $(TREESITTER_CATALA_LANG)/src/parser.c -o $@

# src/scanner.c is scanner.c with the language's keyword table,
# scanner_<lang>.h, in place of its #include "scanner_lang.h", and entry points
# named after the grammar
generate: Cargo.toml.in grammar.js scanner.c scanner_$(TREESITTER_CATALA_LANG).h ALWAYS
	@echo "Generating tree-sitter-$(TREESITTER_CATALA_VARIANT)-$(TREESITTER_CATALA_LANG) (ABI $(ABI)) in $(OUTDIR)"
	@mkdir -p $(OUTDIR)/src
	@sed 's/$${TREESITTER_CATALA_VARIANT}/'$(TREESITTER_CATALA_VARIANT)'/g; s/$${TREESITTER_CATALA_LANG}/'$(TREESITTER_CATALA_LANG)'/g' $< > $(OUTDIR)/Cargo.toml
	@cp grammar.js $(OUTDIR)
	@sed -e '/^#include "scanner_lang.h"$$/{r scanner_$(TREESITTER_CATALA_LANG).h' -e 'd' -e '}' \
	  -e 's/tree_sitter_catala_external_scanner_/tree_sitter_$(SUBDIR)_external_scanner_/' \
	  scanner.c > $(OUTDIR)/src/scanner.c
	@cd $(OUTDIR) && tree-sitter generate --abi=$(ABI)
	@./check_scanner.sh $(TREESITTER_CATALA_LANG) $(OUTDIR)

# ABI 15 parsers of every variant and language, outside the committed
# directories
abi15: ALWAYS
	@for v in $(VARIANTS); do for l in $(LANGS); do \
	  $(MAKE) --no-print-directory generate ABI=15 OUTDIR=$(ABI15_DIR)/$${v}_$$l \
	    TREESITTER_CATALA_VARIANT=$$v TREESITTER_CATALA_LANG=$$l || exit 1; \
	done; done

# Runs the corpus tests (catala_<lang>/test/corpus) of every language that has
# one, rebuilding the parser: the tree-sitter CLI caches the library of a
# grammar by name only. The cases named "... (ABI 15)" need reserved words and
# are skipped.
test: ALWAYS
	@for l in $(LANGS); do \
	  if [ -d catala_$$l/test/corpus ]; then \
	    echo "Testing catala_$$l"; \
	    (cd catala_$$l && tree-sitter test --rebuild --exclude ' \(ABI 15\)$$') \
	      || exit 1; \
	  fi; \
	done

# Runs the same corpus tests on ABI 15 parsers of catala_<lang>, generated in
# $(ABI15_DIR), skipping the cases named "... (ABI 14)"
test-abi15: ALWAYS
	@for l in $(LANGS); do \
	  if [ -d catala_$$l/test/corpus ]; then \
	    d=$(ABI15_DIR)/catala_$$l; \
	    $(MAKE) --no-print-directory generate ABI=15 OUTDIR=$$d \
	      TREESITTER_CATALA_VARIANT=catala TREESITTER_CATALA_LANG=$$l || exit 1; \
	    rm -rf $$d/test && mkdir -p $$d/test && cp -R catala_$$l/test/corpus $$d/test/; \
	    printf '{"grammars":[{"name":"catala_%s","scope":"source.catala_%s","path":".","file-types":["catala_%s"]}],"metadata":{"version":"1.0.0"}}\n' \
	      $$l $$l $$l > $$d/tree-sitter.json; \
	    echo "Testing catala_$$l (ABI 15)"; \
	    (cd $$d && tree-sitter test --rebuild --exclude ' \(ABI 14\)$$') || exit 1; \
	  fi; \
	done

# Checks that the committed catala_expr_<lang> parsers group operators as the
# Catala compiler does (test/precedence/compare.py), with the compiler binary
# in $(CATALA)
CATALA ?= catala
check-precedence: ALWAYS
	@for l in $(LANGS); do \
	  echo "Comparing the precedence of catala_expr_$$l with $(CATALA)"; \
	  CATALA=$(CATALA) ./test/precedence/compare.py catala_expr_$$l test/precedence/$$l.json \
	    || exit 1; \
	done

# Checks that the reserved words of grammar.js are the words that the compiler
# $(CATALA) never accepts as identifiers, taking the candidate words from the
# lexer tables of the compiler's checkout $(CATALA_SRC), and that the ABI 15
# parsers reject exactly those words (reserved_words.py). `reserved_words.py
# update` regenerates the table.
check-reserved: abi15
	./reserved_words.py check --catala $(CATALA) --src $(CATALA_SRC) \
	  --abi15 $(ABI15_DIR) --langs $(LANGS)

dist-clean:
	rm -rf $(LANGS) $(LIBS) $(VARIANTS)_?? $(ABI15_DIR)
