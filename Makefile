TREESITTER_CATALA_LANG ?= en
export TREESITTER_CATALA_LANG

TREESITTER_CATALA_VARIANT ?= catala
export TREESITTER_CATALA_VARIANT

SUBDIR = $(TREESITTER_CATALA_VARIANT)_$(TREESITTER_CATALA_LANG)

LANGS = en fr pl

VARIANTS = catala catala_code catala_expr

COMBINATIONS = $(foreach V,$(VARIANTS),$(foreach L,$(LANGS),$V_$L))

LIB = $(SUBDIR)/libtree-sitter-$(TREESITTER_CATALA_VARIANT)_$(TREESITTER_CATALA_LANG).so

.PHONY: generate lib all test dist-clean ALWAYS

all: $(LANGS)

full: $(VARIANTS)

$(LANGS): Cargo.toml.in grammar.js ALWAYS
	$(MAKE) generate lib TREESITTER_CATALA_LANG=$@

$(VARIANTS): Cargo.toml.in grammar.js ALWAYS
	$(MAKE) $(LANGS) TREESITTER_CATALA_VARIANT=$@

lib: $(LIB)
$(LIB): generate
	@echo "Compiling $@ library"
	@cd $(SUBDIR) && tree-sitter build && tree-sitter build --wasm
	@cp -f $(SUBDIR)/parser.so $(@F)
	# @gcc -shared -fPIC -fno-exceptions -g -O2 -I $(TREESITTER_CATALA_LANG)/src $(TREESITTER_CATALA_LANG)/src/parser.c -o $@

# src/scanner.c is scanner.c with the language's keyword table,
# scanner_<lang>.h, in place of its #include "scanner_lang.h", and entry points
# named after the grammar
generate: Cargo.toml.in grammar.js scanner.c scanner_$(TREESITTER_CATALA_LANG).h ALWAYS
	@echo "Generating tree-sitter-$(TREESITTER_CATALA_VARIANT)-$(TREESITTER_CATALA_LANG)"
	@mkdir -p $(SUBDIR)/src
	@sed 's/$${TREESITTER_CATALA_VARIANT}/'$(TREESITTER_CATALA_VARIANT)'/g; s/$${TREESITTER_CATALA_LANG}/'$(TREESITTER_CATALA_LANG)'/g' $< > $(SUBDIR)/Cargo.toml
	@cp grammar.js $(SUBDIR)
	@sed -e '/^#include "scanner_lang.h"$$/{r scanner_$(TREESITTER_CATALA_LANG).h' -e 'd' -e '}' \
	  -e 's/tree_sitter_catala_external_scanner_/tree_sitter_$(SUBDIR)_external_scanner_/' \
	  scanner.c > $(SUBDIR)/src/scanner.c
	@cd $(SUBDIR) && tree-sitter generate --abi=14 # emacs requires <15
	@./check_scanner.sh $(TREESITTER_CATALA_LANG) $(SUBDIR)

# Runs the corpus tests (catala_<lang>/test/corpus) of every language that has one
test: ALWAYS
	@for l in $(LANGS); do \
	  if [ -d catala_$$l/test/corpus ]; then \
	    echo "Testing catala_$$l"; \
	    (cd catala_$$l && tree-sitter test) || exit 1; \
	  fi; \
	done

dist-clean:
	rm -rf $(LANGS) $(LIBS) $(VARIANTS)_??
