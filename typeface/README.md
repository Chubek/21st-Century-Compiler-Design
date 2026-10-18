# Bundled typefaces

The book loads these unmodified static font files directly from this directory,
including regular, bold, italic, and bold italic faces. No system installation,
font cache update, or network access is needed when building.

| Directory | Family | Version | Official download |
| --- | --- | --- | --- |
| `source-serif/` | Source Serif 4 | 4.005R | [Adobe desktop release](https://github.com/adobe-fonts/source-serif/releases/download/4.005R/source-serif-4.005_Desktop.zip) |
| `source-sans/` | Source Sans 3 | 3.052R | [Adobe OTF release](https://github.com/adobe-fonts/source-sans/releases/download/3.052R/OTF-source-sans-3.052R.zip) |
| `jetbrains-mono/` | JetBrains Mono | 2.304 | [JetBrains release](https://github.com/JetBrains/JetBrainsMono/releases/download/v2.304/JetBrainsMono-2.304.zip) |

Each family is distributed under the SIL Open Font License 1.1. Its original
license and copyright notice are included alongside its fonts (`LICENSE.md` or
`OFL.txt`). Source Sans's OTF archive omits the license, so its license comes
from the [matching release tag](https://github.com/adobe-fonts/source-sans/blob/3.052R/LICENSE.md).

From the repository root:

```sh
texlua build.lua fonts   # Verify all 12 required font files.
texlua build.lua         # Build index.pdf with XeLaTeX.
# Or: ENGINE=lualatex texlua build.lua
```

`build.lua` checks the font files before invoking TeX and reports missing paths.
`support/21st-century-cc-book.cls` maps each family and style to its filename,
so installed fonts cannot silently replace the bundled versions. The existing
`monoligatures` class option is still supported.
