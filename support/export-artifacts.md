# Manuscript artifact exporter

`export-artifacts.py` uses Python 3.10+ and its standard library. It does not run
TeX or require third-party packages.

```sh
# Regenerate the three export directories and their inventories.
python3 support/export-artifacts.py

# Verify content and inventory without writing anything.
python3 support/export-artifacts.py --check

# Inspect a candidate export outside the repository.
python3 support/export-artifacts.py --output-root /tmp/opencode/export-test
python3 support/export-artifacts.py --output-root /tmp/opencode/export-test --check

# Focused tests, including exact source preservation and inventory protection.
PYTHONDONTWRITEBYTECODE=1 python3 -m unittest discover -s support -p 'test_export_artifacts.py' -v
```

`--root PATH` selects another manuscript directory; otherwise the root is the
parent of this script's directory. Output defaults to the selected root.
`--strict` makes conversion warnings fatal before writing. Exit statuses are
0 for success, 1 for inventory/content discrepancies or output conflicts, and
2 for invalid/incomplete input, strict-mode warnings, or an I/O error.

## Reachability and provenance

The exporter starts at `index.ltx` and recursively follows literal, braced
`\include{...}` and `\input{...}` commands. This includes chapter fragments and
figure files. Build-root-relative paths take precedence over paths relative to
the including file. Comments, inline `\verb`, and literal listing environments
cannot introduce phantom inputs or floats. `\endinput` ends a source file.

Unreachable chapters are excluded. Paths outside the manuscript root, `sources/`,
`book/`, and the three generated export directories are rejected if reached.
Missing inputs, input cycles, unbalanced environments, duplicate export labels,
and unsafe label filenames are errors. Every input is reread before output
preflight to detect concurrent edits. Wait for manuscript edits to finish before
regenerating the repository exports; there is no filesystem-wide snapshot lock.

Each artifact records its source file and exact start line, label, and caption.
Its filename is the label suffix: `alg:example` becomes
`pseudo-code/example.sudo`, `fig:example` becomes
`graphs-code/example.tikz.tex`, and `tab:example` becomes
`tables-code/example.md`. Inventories and README tables are generated from the
whole reachable corpus, sorted by filename, without timestamps.

## Representations

- **Algorithms:** uppercase control keywords and two-space structured indentation.
  Nested conditions, loops, functions, procedures, repeat/until, comments, and
  multi-line statements are supported. Fractions retain explicit numerator and
  denominator grouping; subscripts and exponents retain grouping; shared dataflow
  macros become forms such as `In[B]`, `MEET`, and `phi(...)`. Unsupported inline
  commands and their arguments remain visible as LaTeX, with diagnostics. Original
  algorithm source follows as `# `-prefixed lines; removing that prefix and the
  export's final newline reconstructs the original environment exactly.
- **Graphs:** the original `tikzpicture` environment, including options, comments,
  and whitespace, is copied exactly. Surrounding metadata and a `% `-prefixed
  original figure trace record the caption and wrapper. Graphs depend on the
  manuscript's TikZ setup and shared styles. Inputs inside a TikZ body remain
  original `\input` directives; exports are source extracts, not standalone PDFs.
- **Tables:** genuine GFM header, separator, and data rows. Markup, HTML, and pipes
  are escaped rather than interpreted as Markdown or additional cells. The first
  source row is the header. `l`, `c`, `r`, `p`, `m`, `b`, repeated column specs,
  intercolumn material, and column declarations are understood. Grouped cell line
  breaks become ` / `; they do not split the outer row. `\multicolumn` fills its
  first column and leaves subsequent spanned columns empty because GFM has no
  column spans. Common nested math environments retain named, readable forms.
  The exact original table environment follows in a safely sized LaTeX fence.

The plain-text representation is ASCII; unfamiliar Unicode is retained using
explicit Unicode escapes. Exact traces and TikZ bodies retain original Unicode.
No prose or math is silently discarded when an inline command is unknown.

## Supported source contract and limitations

This is a bounded source reader, not a TeX macro interpreter. Each `algorithm`,
`figure`, or `table` float (including starred variants) needs one matching
`alg:`, `fig:`, or `tab:` label, one caption, and one `pseudocode`, `tikzpicture`,
or `tabular` body respectively. A body can come from an input file; its opening
and closing environment commands must reside in the same physical file.
Orphan pseudocode/TikZ environments and multiple bodies in one float are errors.

Macro-generated input paths, macro-generated floats, conditional compilation,
custom column types, `tabularx`, `longtable`, and `multirow` layout are not
interpreted. Use explicit source environments for exportable artifacts.
Unsupported body environments and column structures fail explicitly; unknown
inline commands and extra float wrapper content remain in the original trace
and generate warnings. `--strict` rejects those warnings. TeX `\includeonly`
does not restrict this whole-manuscript export.

Legacy unbraced algorithm conditions are serialized through the next statement
with a warning; use braced conditions for unambiguous exports and correct TeX
argument binding.

## Output ownership

The exporter validates and converts the entire corpus before writing. It
recognizes its own generated marker and the repository's existing legacy export,
README, and manifest formats. A conflicting unrecognized file or symlink aborts
the operation before any writes. Unrecognized export-extension files are also
reported as inventory conflicts and preserved; unrelated auxiliary files are
left alone.

Normal regeneration removes only obsolete **recognized generated** exports.
`--check` reports missing, stale, obsolete, and unrecognized export files plus
README/manifest discrepancies, without modifying anything. Repeating normal
regeneration does not rewrite already matching files. Conversion/preflight is
all-or-nothing; a filesystem write failure can still leave a partially updated
output tree, so rerun after resolving the I/O error.
