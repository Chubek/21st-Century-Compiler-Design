"""Focused converter, provenance, and inventory checks; no TeX installation needed."""

import contextlib
import importlib.util
import io
from pathlib import Path
import sys
import tempfile
import unittest


SPEC = importlib.util.spec_from_file_location("export_artifacts", Path(__file__).with_name("export-artifacts.py"))
export = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = export
SPEC.loader.exec_module(export)


class TextConversionTests(unittest.TestCase):
    def setUp(self):
        self.converter = export.Converter()

    def test_nested_arguments_and_short_caption(self):
        source = r"\caption[Short {nested}]{A \code{f{(x)}} and $\frac{a+{b}}{\frac{c}{d}}$.}\label{alg:test}"
        caption, = export.arguments(source, "caption")
        self.assertEqual(self.converter.plain(caption), "A f(x) and (a+b)/((c)/(d)).")
        self.assertFalse(self.converter.warnings)

    def test_multiline_math_shared_macros_and_grouping(self):
        self.assertEqual(self.converter.plain(r"""$\Out{B} \gets \Gen{B} \cup
          (\In{B} \setminus \Kill{B})$, $x_{i+1}^{2} = \frac{a + b}{c}$"""),
                         r"Out[B] <- Gen[B] UNION (In[B] \ Kill[B]), x_{i+1}^{2} = (a + b)/(c)")

    def test_unknown_macro_survives_with_arguments_and_warning(self):
        self.assertEqual(self.converter.plain(r"$\unknown{x_{i}}[mode] + 1$"),
                         r"\unknown{x_{i}}[mode] + 1")
        self.assertIn(r"retained unsupported inline command \unknown", self.converter.warnings)

    def test_comments_escaped_percent_and_inline_verbatim(self):
        self.assertEqual(self.converter.plain("a \\% b % discarded\n c"), "a % b c")
        self.assertEqual(self.converter.plain("joined% discarded\nword"), "joinedword")
        self.assertEqual(self.converter.plain(r"\verb|a%_\foo| and x"), r"a%_\foo and x")
        text = "% \\input{bad}\n\\verb|\\begin{figure}|\n\\begin{lstlisting}\n\\input{bad}\n\\end{lstlisting}\n"
        self.assertNotIn("input", export.mask(text))
        self.assertEqual(len(export.mask(text)), len(text))

    def test_nested_algorithm_controls_and_multiline_conditions(self):
        body = r"""
\Require $x$
\Function{Choose}{$x$}
\If{$x > \frac{a + b}{c}$ and
    $x \neq 0$}
  \While{$x > 0$}
    \State $x \gets x - 1$ \Comment{keep {counting}}
    \If{$x = 2$}\State \Return $x$\Else\State \Continue\EndIf
  \EndWhile
\ElsIf{$x = 0$}
  \State \Return $0$
\Else
  \State \Return $-1$
\EndIf
\EndFunction
"""
        self.assertEqual(self.converter.pseudocode(body), "\n".join([
            "REQUIRE x", "FUNCTION Choose(x)", "  IF x > (a + b)/(c) and x != 0 THEN",
            "    WHILE x > 0 DO", "      x <- x - 1 // keep counting", "      IF x = 2 THEN",
            "        RETURN x", "      ELSE", "        CONTINUE", "      END IF", "    END WHILE",
            "  ELSE IF x = 0 THEN", "    RETURN 0", "  ELSE", "    RETURN -1", "  END IF", "END FUNCTION"]))

    def test_repeat_and_mismatched_controls(self):
        self.assertEqual(self.converter.pseudocode(r"\Repeat\State work\Until{$done$}"),
                         "REPEAT\n  work\nUNTIL done")
        with self.assertRaisesRegex(export.ExportError, "mismatched"):
            self.converter.pseudocode(r"\If{$x$}\EndWhile")
        with self.assertRaisesRegex(export.ExportError, "unclosed"):
            self.converter.pseudocode(r"\If{$x$}\State work")

    def test_legacy_unbraced_condition_is_retained_and_flagged(self):
        self.assertEqual(self.converter.pseudocode(r"\ForAll blocks $B$\State visit $B$\EndFor"),
                         "FORALL blocks B DO\n  visit B\nEND FOR")
        self.assertTrue(any("unbraced" in warning for warning in self.converter.warnings))

    def test_p_columns_grouped_line_breaks_and_pipes(self):
        source = r"""[t]{@{}p{0.3\linewidth}>{\raggedright}p{6cm}@{}}
\toprule
Name & Value \\
\midrule
\code{a|b_*} & {line one\\line two} \\
\verb|100%| & $\frac{a + b}{c}$ \\
\bottomrule
"""
        count, rows = export.table_rows(source)
        self.assertEqual(count, 2)
        self.assertEqual(len(rows), 3)
        self.assertEqual(self.converter.plain(rows[1][1]), "line one / line two")
        self.assertEqual(export.markdown_cell(self.converter.plain(rows[1][0])), r"a\|b\_\*")
        self.assertEqual(self.converter.plain(rows[2][0]), "100%")
        self.assertEqual(self.converter.plain(rows[2][1]), "(a + b)/(c)")

    def test_gfm_html_and_escape_safety(self):
        self.assertEqual(export.markdown_cell(r"<script> & `x|y` \q [a]"),
                         r"&lt;script&gt; &amp; \`x\|y\` \\q \[a\]")

    def test_aligned_math_does_not_split_table_rows(self):
        _, rows = export.table_rows(r"""{lp{4cm}}
Name & Formula \\
x & $\begin{aligned}a &= b \\
c &= \frac{d}{e}\end{aligned}$ \\
""")
        self.assertEqual(len(rows), 2)
        self.assertEqual(self.converter.plain(rows[1][1]), "aligned(a &= b / c &= (d)/(e))")

    def test_column_repetition_multicolumn_and_invalid_structure(self):
        self.assertEqual(export.column_count(r"@{}l*{2}{p{1cm}r}|"), 5)
        _, rows = export.table_rows(r"{lll} A & B & C \\ \multicolumn{2}{l}{Both} & Last \\")
        self.assertEqual(rows[1], [r"\multicolumn{2}{l}{Both}", "", "Last"])
        with self.assertRaisesRegex(export.ExportError, "expected 2"):
            export.table_rows(r"{ll} A & B & C \\")
        with self.assertRaisesRegex(export.ExportError, "unsupported tabular"):
            export.column_count("Xl")
        with self.assertRaisesRegex(export.ExportError, "unsupported table cell environment"):
            export.table_rows(r"{l}\begin{mystery}x\end{mystery}\\")


ALGORITHM = r"""\begin{algorithm}[ht]
  % This comment and all indentation must survive exactly.
  \caption[Short]{Nested \code{f{(x)}} with $\frac{a}{b}$.}\label{alg:test}
  \begin{pseudocode}
    \Require $x \neq 0$
    \If{$x > 0$}
      \State \Return $x$ % original comment
    \Else
      \State \Return $0$
    \EndIf
  \end{pseudocode}
\end{algorithm}"""
TIKZ = r"""\begin{tikzpicture}[node distance=1cm]
  % Do not normalize whitespace or comments: 100% literal.
  \node[basic block] (a) {a | b \\ \{c\}};
  \draw[cfg edge] (a) -- ++(1,0); % exact trailing comment
\end{tikzpicture}"""
TABLE = r"""\begin{table}
\caption{Pipes and \code{nested{ braces}}.}\label{tab:test}
\centering
\begin{tabular}{lp{5cm}}
\toprule
Name & Value \\
\midrule
\code{x|y_*} & $\frac{a}{b}$ \\
\bottomrule
\end{tabular}
\end{table}"""


class CorpusTests(unittest.TestCase):
    def setUp(self):
        preferred = Path("/tmp/opencode")
        self.temp = tempfile.TemporaryDirectory(prefix="export-artifacts-",
                                                dir=preferred if preferred.is_dir() else None)
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.write("index.ltx", "\\begin{document}\n\\include{chapters/main}\n\\end{document}\n")
        self.write("chapters/main.tex", "% \\input{missing}\n" + ALGORITHM + "\n"
                   + "\\input{chapters/fragment}\n\\input{figures/graph}\n")
        self.write("chapters/fragment.tex", TABLE)
        self.write("figures/graph.tex", "\\begin{figure}\n\\centering\n\\input{figures/body}\n"
                   + "\\caption{A graph.}\\label{fig:test}\n\\end{figure}")
        self.write("figures/body.tex", TIKZ)
        # Unreachable content must never enter the inventory.
        self.write("chapters/unreachable.tex", ALGORITHM.replace("alg:test", "alg:unreachable"))
        self.write("book/index.ltx", "\\input{missing}")
        self.write("sources/reference.tex", "\\input{missing}")

    def write(self, name, text):
        path = self.root / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text, encoding="utf-8")
        return path

    def plan(self):
        manuscript = export.Manuscript(self.root)
        artifacts, warnings = export.collect(manuscript)
        self.assertEqual(warnings, [])
        return export.expected_files(artifacts)

    def test_reachable_inputs_exact_traces_and_metadata(self):
        plan = self.plan()
        self.assertEqual(len(plan), 9)
        algorithm = plan[Path("pseudo-code/test.sudo")]
        trace = algorithm.split("# Original LaTeX:\n", 1)[1].removesuffix("\n")
        self.assertEqual("\n".join(line[2:] for line in trace.split("\n")), ALGORITHM)
        self.assertIn("# source: chapters/main.tex:2", algorithm)
        graph = plan[Path("graphs-code/test.tikz.tex")]
        body = graph[graph.index("\\begin{tikzpicture}"):].split("\n\n% Original figure", 1)[0]
        self.assertEqual(body, TIKZ)
        self.assertIn("% TikZ source: figures/body.tex:1", graph)
        self.assertIn(r"% \centering", graph)
        table = plan[Path("tables-code/test.md")]
        self.assertIn(r"| x\|y\_\* | (a)/(b) |", table)
        self.assertIn(TABLE, table)
        self.assertNotIn("unreachable", "\n".join(plan.values()))

    def test_inventory_content_and_idempotent_regeneration(self):
        plan = self.plan()
        output = self.root / "output"
        self.assertEqual(export.synchronize(output, plan, False), [])
        before = {p: (output / p).stat().st_mtime_ns for p in plan}
        self.assertEqual(export.synchronize(output, plan, True), [])
        self.assertEqual(export.synchronize(output, plan, False), [])
        self.assertEqual(before, {p: (output / p).stat().st_mtime_ns for p in plan})
        (output / "tables-code/test.md").write_text(plan[Path("tables-code/test.md")] + "stale\n")
        (output / "pseudo-code/test.sudo").unlink()
        obsolete = output / "graphs-code/obsolete.tikz.tex"
        obsolete.write_text("% " + export.MARKER + "\n")
        problems = export.synchronize(output, plan, True)
        self.assertTrue(any("stale: tables-code/test.md" in p for p in problems))
        self.assertTrue(any("missing: pseudo-code/test.sudo" in p for p in problems))
        self.assertTrue(any("obsolete generated file:" in p for p in problems))
        self.assertTrue(obsolete.exists(), "--check must not write or remove")
        self.assertEqual(export.synchronize(output, plan, False), [])
        self.assertFalse(obsolete.exists())
        self.assertEqual(export.synchronize(output, plan, True), [])

    def test_unknown_work_blocks_writes_and_is_never_removed(self):
        plan = self.plan()
        output = self.root / "output"
        unknown = output / "tables-code/test.md"
        unknown.parent.mkdir(parents=True)
        unknown.write_text("human work\n")
        problems = export.synchronize(output, plan, False)
        self.assertTrue(any("unrecognized file would be overwritten" in p for p in problems))
        self.assertEqual(unknown.read_text(), "human work\n")
        self.assertFalse((output / "graphs-code").exists())
        unknown.rename(unknown.with_name("notes.md"))
        self.assertTrue(export.synchronize(output, plan, True))
        self.assertTrue(export.synchronize(output, plan, False))
        self.assertEqual(unknown.with_name("notes.md").read_text(), "human work\n")

    def test_changed_source_and_missing_input_are_errors(self):
        manuscript = export.Manuscript(self.root)
        self.write("figures/body.tex", TIKZ + "\n")
        with self.assertRaisesRegex(export.ExportError, "source changed"):
            manuscript.verify_unchanged()
        self.write("index.ltx", "\\input{missing}")
        with self.assertRaises(FileNotFoundError):
            export.Manuscript(self.root)

    def test_cycles_excluded_tree_and_duplicate_labels(self):
        for filename, message in [("index.ltx", "input cycle"),
                                  ("sources/reference", "excluded input tree"),
                                  ("book/index.ltx", "excluded input tree")]:
            with self.subTest(filename=filename):
                self.write("index.ltx", "\\input{" + filename + "}")
                with self.assertRaisesRegex(export.ExportError, message):
                    export.Manuscript(self.root)
        self.write("index.ltx", ALGORITHM + "\n" + ALGORITHM)
        with self.assertRaisesRegex(export.ExportError, "duplicate export label"):
            export.collect(export.Manuscript(self.root))

    def test_unknown_structure_and_extra_content_are_flagged(self):
        self.write("index.ltx", ALGORITHM.replace(r"\State \Return $x$", r"\begin{mystery}x\end{mystery}"))
        with self.assertRaisesRegex(export.ExportError, "unsupported algorithm environments"):
            export.collect(export.Manuscript(self.root))
        self.write("index.ltx", ALGORITHM.replace(r"\begin{pseudocode}", r"\customlayout{1}\begin{pseudocode}"))
        artifacts, warnings = export.collect(export.Manuscript(self.root))
        self.assertTrue(any("unhandled float wrapper" in w for w in warnings))
        self.assertIn(r"\customlayout{1}", artifacts[0].content)

    def test_literal_listing_cannot_create_phantom_floats(self):
        self.write("index.ltx", "\\begin{lstlisting}\n" + ALGORITHM + "\n\\end{lstlisting}\n" + TABLE)
        artifacts, _ = export.collect(export.Manuscript(self.root))
        self.assertEqual([a.label for a in artifacts], ["tab:test"])

    def test_check_cli_is_nonwriting_and_returns_nonzero(self):
        output = self.root / "output"
        with contextlib.redirect_stdout(io.StringIO()), contextlib.redirect_stderr(io.StringIO()):
            self.assertEqual(export.main(["--root", str(self.root), "--output-root", str(output), "--check"]), 1)
            self.assertFalse(output.exists())
            self.assertEqual(export.main(["--root", str(self.root), "--output-root", str(output)]), 0)
            self.assertEqual(export.main(["--root", str(self.root), "--output-root", str(output), "--check"]), 0)

    def test_strict_mode_rejects_unknown_notation_before_writing(self):
        self.write("index.ltx", ALGORITHM.replace(r"\Return $x$", r"\Return $\mystery{x}$"))
        output = self.root / "output"
        with contextlib.redirect_stdout(io.StringIO()), contextlib.redirect_stderr(io.StringIO()):
            self.assertEqual(export.main(["--root", str(self.root), "--output-root", str(output), "--strict"]), 2)
        self.assertFalse(output.exists())

    def test_legacy_inventory_recognition(self):
        self.assertTrue(export.recognized(Path("pseudo-code/MANIFEST.txt"), "first.sudo\nsecond.sudo\n"))
        self.assertTrue(export.recognized(Path("graphs-code/MANIFEST.txt"),
                                         "graphs-code manifest: 1 tikz files\nGenerated from: chapters/\n"))
        self.assertTrue(export.recognized(Path("tables-code/MANIFEST.txt"), "# tables-code manifest — 1 tables\n"))
        self.assertFalse(export.recognized(Path("pseudo-code/MANIFEST.txt"), "personal notes\n"))

    def test_symlink_output_is_refused(self):
        output = self.root / "output"
        output.mkdir()
        external = self.root / "external"
        external.mkdir()
        (output / "graphs-code").symlink_to(external, target_is_directory=True)
        self.assertTrue(any("symlink output" in p for p in export.synchronize(output, self.plan(), False)))
        self.assertEqual(list(external.iterdir()), [])


if __name__ == "__main__":
    unittest.main()
