/* The French multi-word keywords of compiler/surface/lexer_fr.cppo.ml, for
   scanner.c. The MR_ regexps join the words with space_plus (SPACES);
   "par ordre croissant" has no MR_ regexp and matches its MS_ string, with
   exactly one space between words (ONE_SPACE). Non-ASCII letters are
   universal character names. */
static const Keyword KEYWORDS[] = {
    /* champ d'application */
    {SCOPE, SPACES, {L"champ", L"d'application"}},
    /* dépend de */
    {DEPENDS, SPACES, {L"dépend", L"de"}},
    {LIST, SPACES, {L"liste", L"de"}},
    {OPTION, SPACES, {L"optionnel", L"de"}},
    /* égal à */
    {DEFINED_AS, SPACES, {L"égal", L"à"}},
    {WILDCARD, SPACES, {L"n'importe", L"quel"}},
    {WITH_PATT, SPACES, {L"sous", L"forme"}},
    {UNDER_CONDITION, SPACES, {L"sous", L"condition"}},
    {WE_HAVE, SPACES, {L"on", L"a"}},
    {MAP_EACH, SPACES, {L"transforme", L"chaque"}},
    {ORDER_INCREASING, ONE_SPACE, {L"par", L"ordre", L"croissant"}},
    /* par ordre décroissant */
    {ORDER_DECREASING, SPACES, {L"par", L"ordre", L"décroissant"}},
    {XOR, SPACES, {L"ou", L"bien"}},
    {OR_EMPTY, SPACES, {L"ou", L"si", L"liste", L"vide"}},
    /* mais en remplaçant */
    {BUT_REPLACE, SPACES, {L"mais", L"en", L"remplaçant"}},
};
