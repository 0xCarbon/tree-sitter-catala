/* The English multi-word keywords of compiler/surface/lexer_en.cppo.ml, for
   scanner.c. The MR_ regexps join the words with space_plus (SPACES); the
   ordering keywords have no MR_ regexp and match their MS_ string, with
   exactly one space between words (ONE_SPACE). */
static const Keyword KEYWORDS[] = {
    {DEPENDS, SPACES, {L"depends", L"on"}},
    {LIST, SPACES, {L"list", L"of"}},
    {OPTION, SPACES, {L"optional", L"of"}},
    {WITH_PATT, SPACES, {L"with", L"pattern"}},
    {UNDER_CONDITION, SPACES, {L"under", L"condition"}},
    {WE_HAVE, SPACES, {L"we", L"have"}},
    {MAP_EACH, SPACES, {L"map", L"each"}},
    {ORDER_INCREASING, ONE_SPACE, {L"in", L"increasing", L"order"}},
    {ORDER_DECREASING, ONE_SPACE, {L"in", L"decreasing", L"order"}},
    {AND_THEN, SPACES, {L"and", L"then"}},
    {OR_EMPTY, SPACES, {L"or", L"if", L"list", L"empty"}},
    {BUT_REPLACE, SPACES, {L"but", L"replace"}},
};
