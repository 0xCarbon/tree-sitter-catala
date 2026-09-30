#!/usr/bin/env python3
"""Compares the operator precedence of a tree-sitter grammar with the Catala
compiler's.

Each case of CASES (a JSON file, one per language) is an expression U with its
two possible parenthesizations OUT and IN, for example `maximum of l + 1`,
`(maximum of l) + 1` and `maximum of (l + 1)`.

- The compiler's parse is the parenthesization whose `catala dcalc` output
  equals U's; when U does not typecheck, it is the one that does not typecheck
  either, the other one typechecking.
- tree-sitter's parse is the parenthesization whose tree, once its e_paren
  nodes are removed, equals U's tree in the catala_expr_<lang> grammar.

Usage: CATALA=<catala binary> compare.py <catala_expr_lang dir> <cases.json>
Exits with 1 if the two parses differ for any case, or the compiler's cannot
be determined.
"""
import json, os, re, subprocess, sys, tempfile

CATALA = os.environ.get("CATALA", "catala")
ts_dir, cases_file = sys.argv[1], sys.argv[2]
spec = json.load(open(cases_file))
work = tempfile.mkdtemp()
rebuild = ["--rebuild"]


def dcalc(ty, expr):
    """The dcalc output of the declaration, None if it does not typecheck."""
    src = "```catala\n" + spec["prelude"] + "\n" + spec["decl"].format(ty=ty, expr=expr) + "\n```\n"
    path = os.path.join(work, "t.catala_" + spec["lang"])
    with open(path, "w") as f:
        f.write(src)
    r = subprocess.run([CATALA, "dcalc", "--no-stdlib", path], capture_output=True,
                       text=True, env={**os.environ, "NO_COLOR": "1"})
    if r.returncode != 0:
        if re.search(r"Syntax error|Parsing error|Lexing error", r.stderr):
            sys.exit(f"syntax error in {expr!r}:\n{r.stderr}")
        return None
    m = re.search(r"let topval r\b.*?(?=\n\n|\Z)", r.stdout, re.S)
    return m.group(0) if m else None


def without_parens(tree):
    """tree with every (e_paren (LPAREN) X (RPAREN)) replaced by X."""
    opening = "(e_paren (LPAREN) "
    while (start := tree.find(opening)) >= 0:
        i = j = start + len(opening)
        depth = 0
        while not (depth == 0 and tree.startswith(" (RPAREN))", j)):
            depth += {"(": 1, ")": -1}.get(tree[j], 0)
            j += 1
        tree = tree[:start] + tree[i:j] + tree[j + len(" (RPAREN))"):]
    return tree


def parse(expr):
    global rebuild
    path = os.path.join(work, "e.txt")
    with open(path, "w") as f:
        f.write(expr + "\n")
    r = subprocess.run(["tree-sitter", "parse", *rebuild, path], cwd=ts_dir,
                       capture_output=True, text=True)
    rebuild = []
    tree = re.sub(r"\s*\[\d+, \d+\] - \[\d+, \d+\]", "", r.stdout)
    tree = re.sub(r"\n.*Parse:.*", "", tree)
    tree = re.sub(r"\w+: ", "", re.sub(r"\s+", " ", tree).strip())
    return without_parens(tree)


def verdict(u, out, inside):
    if u is not None and u == out != inside: return "OUT"
    if u is not None and u == inside != out: return "IN"
    # U does not typecheck: its parse is the parenthesization that does not
    # either, when the other one does
    if u is None and out is None and inside is not None: return "OUT"
    if u is None and inside is None and out is not None: return "IN"
    return "?"


failures = 0
for name, ty, u, out, inside in spec["cases"]:
    compiler = verdict(dcalc(ty, u), dcalc(ty, out), dcalc(ty, inside))
    tu = parse(u)
    ts = "ERR" if re.search(r"\b(ERROR|MISSING)\b", tu) else verdict(tu, parse(out), parse(inside))
    ok = compiler in ("OUT", "IN") and compiler == ts
    failures += not ok
    print(f"{'ok ' if ok else 'BAD'} {name:28} compiler={compiler:3} tree-sitter={ts:3} {u}")
print(f"{failures} mismatch(es)")
sys.exit(1 if failures else 0)
