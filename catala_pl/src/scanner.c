/* External scanner of the Catala grammars.

   `make generate` writes it to <grammar>/src/scanner.c, replacing the
   #include of "scanner_lang.h" with the language's table,
   scanner_<lang>.h, and naming the entry points after the grammar.

   It lexes the multi-word keywords ("and then", "list of", "égal à"...),
   which a tree-sitter regular expression cannot lex the way the compiler's
   lexer does, because they need to look past their own end: in the
   compiler (since "Lex a multi-word keyword running into an identifier as
   its first word"), a multi-word keyword directly followed by an identifier
   character is not that keyword ("a and then_x" is "a", "and" and the
   identifier "then_x"). The scanner reads whole words, so a keyword matches
   only when each of its words is a whole word of the input; otherwise it
   declines and the internal lexer lexes the first word alone, as a keyword
   or an identifier, like the compiler's lex_first_word.

   Identifier characters are ASCII letters and digits, '_', '\'' and every
   non-ASCII character that is not a white space. The compiler's set is the
   Unicode letters with the Lowercase or Uppercase property, digits, '_' and
   '\''; the two sets differ only on non-ASCII characters that the compiler
   rejects in code anyway (symbols, combining marks), so a keyword is
   accepted by this scanner exactly when the compiler accepts it.

   The scanner is stateless. */

#include "tree_sitter/parser.h"

#include <stdbool.h>
#include <stdint.h>
#include <wchar.h>

/* The tokens that the compiler's lexer may lex as a multi-word keyword: the
   MR_ regexps of the rollback rule of lex_code (compiler/surface/lexer.cppo.ml).
   Same order as `scanner_tokens` in grammar.js, which lists them as the
   grammar's externals; `make generate` checks that the two lists agree. In a
   language where one of them is a single word, the language's table has no
   entry for it, the grammar's external at its position is a placeholder that
   is never valid, and the internal lexer lexes the token. */
enum TokenType {
  SCOPE,
  DEPENDS,
  DECREASING,
  INCREASING,
  LIST,
  OPTION,
  DEFINED_AS,
  WILDCARD,
  WITH_PATT,
  UNDER_CONDITION,
  WITH,
  WE_HAVE,
  MAP_EACH,
  TO,
  SUCH,
  ORDER_INCREASING,
  ORDER_DECREASING,
  AND_THEN,
  XOR,
  OR_EMPTY,
  BUT_REPLACE,
};

/* The separator between the words of a keyword */
enum Separator {
  /* space_plus: one or more white spaces, newlines included (an MR_ regexp
     of the compiler) */
  SPACES,
  /* exactly one U+0020 space (an MS_ string without MR_ regexp, which the
     compiler matches literally) */
  ONE_SPACE,
};

#define MAX_WORDS 4
#define MAX_WORD_LEN 32

typedef struct {
  enum TokenType token;
  enum Separator separator;
  const wchar_t *words[MAX_WORDS]; /* NULL-terminated when shorter */
} Keyword;

/* The language's KEYWORDS table */
/* The Polish multi-word keywords of compiler/surface/lexer_pl.cppo.ml, for
   scanner.c. The MR_ regexps join the words with space_plus (SPACES); the
   ordering keywords have no MR_ regexp and match their MS_ string, with
   exactly one space between words (ONE_SPACE). Non-ASCII letters are
   universal character names. */
static const Keyword KEYWORDS[] = {
    /* zależy od */
    {DEPENDS, SPACES, {L"zależy", L"od"}},
    {WITH_PATT, SPACES, {L"ze", L"wzorem"}},
    {UNDER_CONDITION, SPACES, {L"pod", L"warunkiem"}},
    {WITH, SPACES, {L"wraz", L"z"}},
    /* plan każdy */
    {MAP_EACH, SPACES, {L"plan", L"każdy"}},
    /* takie że */
    {SUCH, SPACES, {L"takie", L"że"}},
    {ORDER_INCREASING, ONE_SPACE, {L"by", L"increasing", L"order"}},
    {ORDER_DECREASING, ONE_SPACE, {L"by", L"decreasing", L"order"}},
    {AND_THEN, SPACES, {L"and", L"then"}},
    /* lub jeżeli lista pusta */
    {OR_EMPTY, SPACES, {L"lub", L"jeżeli", L"lista", L"pusta"}},
    /* ale zastąpić */
    {BUT_REPLACE, SPACES, {L"ale", L"zastąpić"}},
};

#define KEYWORD_COUNT (sizeof(KEYWORDS) / sizeof(KEYWORDS[0]))

/* The compiler's white_space (Unicode White_Space) */
static bool is_space(int32_t c) {
  return (c >= 0x09 && c <= 0x0D) || c == 0x20 || c == 0x85 || c == 0xA0 ||
         c == 0x1680 || (c >= 0x2000 && c <= 0x200A) || c == 0x2028 ||
         c == 0x2029 || c == 0x202F || c == 0x205F || c == 0x3000;
}

static bool is_idchar(int32_t c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
         (c >= '0' && c <= '9') || c == '_' || c == '\'' ||
         (c >= 0x80 && !is_space(c));
}

static void advance(TSLexer *lexer) { lexer->advance(lexer, false); }

/* Reads a whole word (the maximal run of identifier characters). Returns its
   length, or MAX_WORD_LEN + 1 when it is longer than any keyword word. */
static unsigned read_word(TSLexer *lexer, int32_t *word) {
  unsigned len = 0;
  while (is_idchar(lexer->lookahead)) {
    if (len < MAX_WORD_LEN) word[len] = lexer->lookahead;
    len++;
    advance(lexer);
  }
  return len > MAX_WORD_LEN ? MAX_WORD_LEN + 1 : len;
}

static bool word_equals(const int32_t *word, unsigned len, const wchar_t *kw) {
  unsigned i = 0;
  for (; i < len; i++)
    if (kw[i] == 0 || (int32_t)kw[i] != word[i]) return false;
  return kw[i] == 0;
}

/* Lexes the longest valid keyword whose words are all whole words */
static bool scan_keyword(TSLexer *lexer, const bool *valid_symbols) {
  bool candidate[KEYWORD_COUNT];
  unsigned remaining = 0;
  for (unsigned k = 0; k < KEYWORD_COUNT; k++) {
    candidate[k] = valid_symbols[KEYWORDS[k].token];
    if (candidate[k]) remaining++;
  }
  if (remaining == 0) return false;

  while (is_space(lexer->lookahead)) lexer->advance(lexer, true);

  bool found = false;
  int32_t word[MAX_WORD_LEN];
  for (unsigned w = 0; w < MAX_WORDS && remaining > 0; w++) {
    if (w > 0) {
      /* The separator: a run of white spaces */
      unsigned spaces = 0;
      bool one_space = lexer->lookahead == ' ';
      while (is_space(lexer->lookahead)) {
        spaces++;
        advance(lexer);
      }
      if (spaces == 0) break;
      one_space = one_space && spaces == 1;
      for (unsigned k = 0; k < KEYWORD_COUNT; k++)
        if (candidate[k] && KEYWORDS[k].separator == ONE_SPACE && !one_space) {
          candidate[k] = false;
          remaining--;
        }
      if (remaining == 0) break;
    }
    unsigned len = read_word(lexer, word);
    if (len == 0) break;
    remaining = 0;
    for (unsigned k = 0; k < KEYWORD_COUNT; k++) {
      if (!candidate[k]) continue;
      const wchar_t *kw = KEYWORDS[k].words[w];
      if (kw == NULL || !word_equals(word, len, kw)) {
        candidate[k] = false;
      } else if (w + 1 == MAX_WORDS || KEYWORDS[k].words[w + 1] == NULL) {
        /* Complete keyword: keep it unless a longer one also matches */
        candidate[k] = false;
        lexer->result_symbol = KEYWORDS[k].token;
        lexer->mark_end(lexer);
        found = true;
      } else {
        remaining++;
      }
    }
  }
  return found;
}

void *tree_sitter_catala_pl_external_scanner_create(void) { return NULL; }

void tree_sitter_catala_pl_external_scanner_destroy(
    void *payload) {
  (void)payload;
}

unsigned tree_sitter_catala_pl_external_scanner_serialize(
    void *payload, char *buffer) {
  (void)payload;
  (void)buffer;
  return 0;
}

void tree_sitter_catala_pl_external_scanner_deserialize(
    void *payload, const char *buffer, unsigned length) {
  (void)payload;
  (void)buffer;
  (void)length;
}

bool tree_sitter_catala_pl_external_scanner_scan(
    void *payload, TSLexer *lexer, const bool *valid_symbols) {
  (void)payload;
  return scan_keyword(lexer, valid_symbols);
}
