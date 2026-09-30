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
