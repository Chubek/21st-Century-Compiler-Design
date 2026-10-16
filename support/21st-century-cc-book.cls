%% ---------------------------------------------------------------
%% 21st-century-cc-book.cls
%% Document class for a book on compilers and interpreters.
%%
%% Requires XeLaTeX or LuaLaTeX (fontspec).
%% Fonts (SIL OFL, install system-wide):
%%   Source Serif 4   body text
%%   Source Sans 3    headings, captions, running heads
%%   JetBrains Mono   code
%%
%% Options:
%%   a4paper (default) | letterpaper
%%   10pt | 11pt (default) | 12pt
%%   oneside           (default is twoside, chapters open on right pages)
%%   monoligatures     keep JetBrains Mono's code ligatures (off by default)
%%
%% Extra commands and environments:
%%   \subtitle{...}    used by \maketitle
%%   \code{...}        inline monospace
%%   \term{...}        emphasised first use, also added to the index
%%   ccnote, ccwarning  boxed callouts: \begin{ccnote}[Optional title]
%% ---------------------------------------------------------------
\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{21st-century-cc-book}[2026/10/15 v1.0 Compiler book class]

% ---------- Engine check ----------
\RequirePackage{iftex}
\ifXeTeX\else\ifLuaTeX\else
  \ClassError{21st-century-cc-book}%
    {This class requires XeLaTeX or LuaLaTeX}%
    {Compile with `xelatex' or `lualatex' so that fontspec can load OpenType fonts.}%
\fi\fi

% ---------- Options ----------
\newif\ifcc@oneside
\newif\ifcc@monoligs
\def\cc@paper{a4paper}
\def\cc@ptsize{11pt}

\DeclareOption{a4paper}{\def\cc@paper{a4paper}}
\DeclareOption{letterpaper}{\def\cc@paper{letterpaper}}
\DeclareOption{10pt}{\def\cc@ptsize{10pt}}
\DeclareOption{11pt}{\def\cc@ptsize{11pt}}
\DeclareOption{12pt}{\def\cc@ptsize{12pt}}
\DeclareOption{oneside}{\cc@onesidetrue}
\DeclareOption{monoligatures}{\cc@monoligstrue}
\DeclareOption*{\PassOptionsToClass{\CurrentOption}{book}}
\ProcessOptions\relax

\ifcc@oneside
  \LoadClass[\cc@ptsize,oneside]{book}
\else
  \LoadClass[\cc@ptsize,twoside,openright]{book}
\fi

% ---------- Page geometry ----------
\RequirePackage[
  \cc@paper,
  inner=28mm, outer=22mm,
  top=28mm, bottom=30mm,
  headheight=14pt
]{geometry}

% ---------- Fonts ----------
\RequirePackage{fontspec}
\setmainfont{Source Serif 4}
\setsansfont{Source Sans 3}
\ifcc@monoligs
  \setmonofont{JetBrains Mono}[Scale=MatchLowercase]
\else
  % JetBrains Mono builds its ligatures from `calt', so switch that off too.
  \setmonofont{JetBrains Mono}[
    Scale=MatchLowercase,
    Ligatures=NoCommon,
    RawFeature={-calt}
  ]
\fi

% ---------- Language and typography ----------
\RequirePackage{polyglossia}
\setdefaultlanguage{english}
\RequirePackage{microtype}
% Give long inline mathematics room to break paragraphs without margin overflow.
\setlength{\emergencystretch}{2em}
\RequirePackage{setspace}
\setstretch{1.08}
\RequirePackage{amsmath,amssymb}
\RequirePackage{booktabs}
\RequirePackage{enumitem}
\RequirePackage{graphicx}
\graphicspath{{figures/}}
\RequirePackage{xcolor}

\definecolor{ccaccent}{HTML}{1F4E8C}
\definecolor{ccwarn}{HTML}{A33A1F}
\definecolor{codebg}{gray}{0.96}
\definecolor{codecomment}{rgb}{0.30,0.45,0.30}

% ---------- Headings (sans) ----------
\RequirePackage{titlesec}
\titleformat{\chapter}[display]
  {\sffamily\bfseries\color{ccaccent}}
  {\Large\chaptertitlename\ \thechapter}{12pt}{\Huge\raggedright\color{black}}
\titleformat{name=\chapter,numberless}[display]
  {\sffamily\bfseries}{}{0pt}{\Huge}
\titlespacing*{\chapter}{0pt}{40pt}{30pt}
\titleformat{\section}{\sffamily\bfseries\Large}{\thesection}{1em}{}
\titleformat{\subsection}{\sffamily\bfseries\large}{\thesubsection}{1em}{}
\titleformat{\subsubsection}{\sffamily\bfseries}{\thesubsubsection}{1em}{}

% ---------- Running headers ----------
\RequirePackage{fancyhdr}
\renewcommand{\chaptermark}[1]{\markboth{\thechapter.\ #1}{}}
\renewcommand{\sectionmark}[1]{\markright{\thesection\ #1}}
\pagestyle{fancy}
\fancyhf{}
\fancyhead[LE]{\sffamily\small\thepage\quad\leftmark}
\fancyhead[RO]{\sffamily\small\rightmark\quad\thepage}
\renewcommand{\headrulewidth}{0.3pt}
\fancypagestyle{plain}{%
  \fancyhf{}\fancyfoot[C]{\sffamily\small\thepage}%
  \renewcommand{\headrulewidth}{0pt}}

% ---------- Captions ----------
\RequirePackage[font={sf,small},labelfont=bf,labelsep=period]{caption}

% ---------- Source and IR listings (minted is part of standard TeX Live) ----------
% Build only trusted input: the driver enables shell escape for Pygments.
\RequirePackage[newfloat]{minted}
\setminted{
  fontsize=\small, linenos, numbersep=8pt,
  frame=lines, framesep=3mm, bgcolor=codebg,
  breaklines=false, tabsize=4
}
% fvextra, loaded by minted, must precede csquotes.
\RequirePackage{csquotes}

% Keep the public listings/irlisting interfaces for compatibility.
% New non-pseudocode listings use minted or inputminted.
\RequirePackage{listings}
\lstset{
  basicstyle=\ttfamily\small,
  keywordstyle=\color{ccaccent}\bfseries,
  commentstyle=\color{codecomment}\itshape,
  stringstyle=\color{ccwarn},
  backgroundcolor=\color{codebg},
  frame=single, rulecolor=\color{black!15},
  columns=fixed, keepspaces=true,
  breaklines=true, tabsize=4,
  showstringspaces=false,
  numbers=left, numberstyle=\tiny\sffamily\color{black!50}, numbersep=8pt,
  xleftmargin=1.6em, framexleftmargin=1.2em
}
\newcommand{\code}[1]{\texttt{#1}}

% ---------- Callout boxes ----------
\RequirePackage[most]{tcolorbox}
\tcbset{
  cc@base/.style={
    enhanced, breakable, sharp corners, boxrule=0.4pt,
    left=6pt, right=6pt, top=4pt, bottom=4pt,
    fonttitle=\sffamily\bfseries, before upper={\setlength{\parindent}{0pt}}
  }
}
\newtcolorbox{ccnote}[1][Note]{%
  cc@base, title={#1},
  colback=ccaccent!4, colframe=ccaccent}
\newtcolorbox{ccwarning}[1][Warning]{%
  cc@base, title={#1},
  colback=ccwarn!4, colframe=ccwarn}

% ---------- Index ----------
\RequirePackage{makeidx}
\makeindex
\newcommand{\term}[1]{\emph{#1}\index{#1}}

% ---------- Bibliography (add \addbibresource in the document) ----------
\RequirePackage[backend=biber,style=numeric,sorting=nyt]{biblatex}

% ---------- Title page ----------
\gdef\cc@subtitle{}
\newcommand{\subtitle}[1]{\gdef\cc@subtitle{#1}}
\renewcommand{\maketitle}{%
  \begin{titlepage}
    \centering
    \vspace*{0.18\textheight}
    {\sffamily\bfseries\fontsize{30}{34}\selectfont\color{ccaccent}\@title\par}
    \ifx\cc@subtitle\@empty\else
      \vskip 1.2em
      {\sffamily\Large\cc@subtitle\par}
    \fi
    \vskip 4em
    {\sffamily\Large\@author\par}
    \vfill
    {\sffamily\@date\par}
  \end{titlepage}%
}

% ---------- Hyperref (load last) ----------
\RequirePackage[hidelinks,bookmarksnumbered]{hyperref}

\endinput
