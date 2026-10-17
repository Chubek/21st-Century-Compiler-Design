"""End-to-end C99 generation tests; Python 3 stdlib, Perl, GCC, Clang.
All generated files and binaries live in /tmp/opencode, never the source tree.
"""
from pathlib import Path
import itertools
import os
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parent.parent


class AurocksTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.TemporaryDirectory(prefix="aurocks-", dir="/tmp/opencode")
        cls.work = Path(cls.tmp.name)
        cls.serial = 0

    @classmethod
    def tearDownClass(cls):
        cls.tmp.cleanup()

    def generate(self, grammar, extra=()):
        type(self).serial += 1
        path = self.work / f"g{self.serial}.g"
        path.write_text(grammar)
        return subprocess.run(["perl", str(ROOT / "support/aurocks.pl"), *extra, str(path)],
                              capture_output=True, text=True, timeout=20)

    def compile_run(self, grammar, args=(), expected=0):
        result = self.generate(grammar)
        self.assertEqual(result.returncode, 0, result.stderr)
        source = self.work / f"g{self.serial}.c"
        source.write_text(result.stdout)
        outputs = []
        for cc, sanitize in itertools.product(("gcc", "clang"), (False, True)):
            exe = source.with_suffix(f".{cc}{'-san' if sanitize else ''}")
            flags = ["-std=c99", "-Wall", "-Wextra", "-Wpedantic", "-Werror"]
            if sanitize:
                flags += ["-fsanitize=address,undefined", "-fno-omit-frame-pointer"]
            subprocess.run([cc, *flags, str(source), "-o", str(exe)], check=True, capture_output=True)
            run = subprocess.run([str(exe), *args], capture_output=True, text=True, timeout=30,
                                 env={**os.environ, "ASAN_OPTIONS": "detect_leaks=1"})
            self.assertEqual(run.returncode, expected, run.stderr)
            self.assertEqual(run.stderr, "", run.stderr)
            outputs.append(run.stdout)
        self.assertEqual(len(set(outputs)), 1)
        return outputs[0]

    @staticmethod
    def grammar(body, main, header="", prologue=""):
        return "%{\n#include <assert.h>\n#include <stdio.h>\n" + prologue + "\n%}\n%start S\n" + header + "\n%%\n" + body + "\n%%\n" + main

    def test_caller_backtracking_and_deferred_actions(self):
        g = (ROOT / "example-code/aurocks/backtrack.g").read_text()
        self.assertEqual(self.compile_run(g, ["ab"]), "0 2 2\n")
        self.assertEqual(self.compile_run(g, ["abb"]), "0 2 3\n")
        self.assertEqual(self.compile_run(g, ["abx"]), "1 0 2\n")

    def test_calculator(self):
        g = (ROOT / "example-code/aurocks/calculator.g").read_text()
        self.assertEqual(self.compile_run(g, ["2+3*(4+1)"]), "17\n")

    def test_bounded_language_oracle_and_budgets(self):
        # Independent recognition: a^n b^n, compared on every binary word <= 8.
        main = r'''
int main(void) {
  unsigned checked = 0;
  for (size_t n = 0; n <= 8; ++n) {
    for (unsigned bits = 0; bits < (1U << n); ++bits) {
      char s[9];
      for (size_t i = 0; i < n; ++i) s[i] = bits & (1U << i) ? 'a' : 'b';
      size_t a = 0; while (a < n && s[a] == 'a') ++a;
      int oracle = n == 2 * a;
      for (size_t i = a; i < n; ++i) if (s[i] != 'b') oracle = 0;
      AUResult r = parse_S(s, n, NULL);
      assert((r.status == AU_OK) == oracle);
      assert(r.status == AU_OK || r.status == AU_SYNTAX);
      if (r.status == AU_OK) {
        AULimits exact = {r.steps, 256};
        AUResult e = parse_S(s, n, &exact); assert(e.status == AU_OK); au_result_free(&e);
        --exact.steps;
        e = parse_S(s, n, &exact); assert(e.status == AU_LIMIT); au_result_free(&e);
      }
      au_result_free(&r); ++checked;
    }
  }
  AULimits zero = {0, 256};
  AUResult r = parse_S("", 0, &zero); assert(r.status == AU_LIMIT); au_result_free(&r);
  zero.steps = 10000; zero.depth = 0;
  r = parse_S("ab", 2, &zero); assert(r.status == AU_LIMIT); au_result_free(&r);
  zero.depth = 257;
  r = parse_S("ab", 2, &zero); assert(r.status == AU_INVALID); au_result_free(&r);
  r = parse_S(NULL, 0, NULL); assert(r.status == AU_INVALID); au_result_free(&r);
  printf("%u words checked\n", checked);
  return 0;
}'''
        self.assertEqual(self.compile_run(self.grammar("S: 'a' S 'b' | %empty;", main)), "511 words checked\n")

    def test_regex_endpoints_and_byte_spans(self):
        main = r'''
int main(void) {
 const char *yes[] = {"ab", "aab", "aaab", "aabbb", "z7", "\t z4 \n"};
 const char *no[] = {"", "b", "aaaab", "aabx", "zq", "x\nz7"};
 for (size_t i=0; i<sizeof yes/sizeof *yes; ++i) {
   AUResult r=parse_S(yes[i],strlen(yes[i]),NULL); assert(r.status==AU_OK); au_result_free(&r);
 }
 for (size_t i=0; i<sizeof no/sizeof *no; ++i) {
   AUResult r=parse_S(no[i],strlen(no[i]),NULL); assert(r.status==AU_SYNTAX); au_result_free(&r);
 }
 return 0;
}'''
        self.compile_run(self.grammar(r"S: /a{1,3}b+/ | /z[[:digit:]]/;", main, r"%skip /[ \t\r\n]+/"))
        self.compile_run(self.grammar(r"S: /.+$/ { assert($TOKEN_LEN == 3); assert($TOKEN[1] == 0); $$ = $1; };", r'''
int main(void) { char s[3]={'a',0,'b'}; AUResult r=parse_S(s,3,NULL);
 assert(r.status==AU_OK); au_result_free(&r); return 0; }'''))

    def test_atomic_longest_terminal(self):
        self.compile_run(self.grammar("S: /a+/ 'a';", r'''
int main(void) { AUResult r=parse_S("aa",2,NULL); assert(r.status==AU_SYNTAX);
 au_result_free(&r); return 0; }'''))

    def test_literal_escaping_and_c_trigraphs(self):
        self.compile_run(self.grammar(r'''S: '??!' '\\' '\'' '\n';''', r'''
int main(void) {
 const char input[] = {'?', '?', '!', '\\', '\'', '\n'};
 AUResult r = parse_S(input, sizeof input, NULL);
 assert(r.status == AU_OK); au_result_free(&r); return 0;
}'''))

    def test_automata_chapter_language(self):
        main = r'''
int main(void) {
 unsigned total=0;
 for (size_t n=0, count=1; n<=6; ++n, count*=4) {
   for (size_t code=0; code<count; ++code) {
     char s[6]; size_t digits=code;
     for (size_t i=0; i<n; ++i) { s[i]="abcd"[digits%4]; digits/=4; }
     int expected=n>=2 && s[0]=='a' && s[n-1]=='d';
     for (size_t i=1; i+1<n; ++i) if (s[i]!='b' && s[i]!='c') expected=0;
     AUResult r=parse_S(s,n,NULL);
     assert(r.status==(expected ? AU_OK : AU_SYNTAX)); au_result_free(&r); ++total;
   }
 }
 printf("%u automata words checked\n",total); return 0;
}'''
        self.assertEqual(self.compile_run(self.grammar("S: /a(b|c)*d/;", main)), "5461 automata words checked\n")

    def test_nested_actions_strings_and_empty_success(self):
        self.compile_run(self.grammar(r'''S: %empty { if (1) { const char *s="$$ $1 } //";
 assert(s[0]=='$'); /* { $99 */ $$ = NULL; } };''', r'''
int main(void) { AUResult r=parse_S("",0,NULL); assert(r.status==AU_OK && !r.value);
 au_result_free(&r); au_result_free(&r); return 0; }'''))

    def test_allocation_failure(self):
        self.compile_run(self.grammar("S: 'x';", r'''
int main(void) { AUResult r=parse_S("x",1,NULL); assert(r.status==AU_NOMEM);
 au_result_free(&r); return 0; }''', prologue=r'''
static void *broken_malloc(size_t n) { (void)n; return NULL; }
#define malloc broken_malloc
'''))

    def test_reject_bad_grammars_without_partial_output(self):
        bodies = ["S: S 'a' | 'b';", "S: A 'b'; A: S | 'a';",
                  "S: A S 'b' | 'a'; A: %empty;", "S: Missing;",
                  "S: /a*/;", "S: 'a' @;", "S: 'a'", "S: %empty 'a';",
                  "S: 'a' { $$ = $2; };", "S: /[z-a]/;", "S: /a**/;",
                  "S: /a{999}/;", "S: 'a' %cut;", "S: ;"]
        for body in bodies:
            with self.subTest(body=body):
                r = self.generate(self.grammar(body, ""))
                self.assertNotEqual(r.returncode, 0)
                self.assertEqual(r.stdout, "")
        for header in ["%glr", "%skip /[ ]*/", "%start S", "%skip /[/"]:
            r = self.generate(self.grammar("S: 'x';", "", header))
            self.assertNotEqual(r.returncode, 0)
            self.assertEqual(r.stdout, "")

    def test_deterministic_output_and_entrypoint(self):
        g = self.grammar("S: 'x';", "")
        a = self.generate(g, ["--entrypoint", "read_input"])
        b = self.generate(g, ["--entrypoint", "read_input"])
        self.assertEqual(a.returncode, 0, a.stderr)
        self.assertEqual(a.stdout, b.stdout)
        self.assertIn("AUResult read_input(", a.stdout)
        for bad in ("int", "au_result_free", "bad-name"):
            r = self.generate(g, ["--entrypoint", bad])
            self.assertNotEqual(r.returncode, 0)
            self.assertEqual(r.stdout, "")


if __name__ == "__main__":
    unittest.main()
