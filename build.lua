#!/usr/bin/env texlua
-- build.lua : build script for the compiler book (index.ltx)
--
-- Usage (works with texlua or plain lua 5.3+):
--   texlua build.lua                 full build (xelatex + biber + makeindex)
--   texlua build.lua draft 1 2 5     build only chapters 1, 2 and 5 (fast)
--   texlua build.lua draftfull 1     partial build with bibliography and index
--   texlua build.lua quick           single xelatex pass, no bib/index
--   texlua build.lua fonts           check that the required fonts are installed
--   texlua build.lua check           check that all chapter files exist
--   texlua build.lua clean           remove auxiliary files
--   texlua build.lua distclean       clean + remove the PDF
--
-- Environment overrides:
--   ENGINE=lualatex texlua build.lua
-- The TeX engine uses shell escape for minted. Build trusted sources only.

local config = {
  main      = "index.ltx",
  jobname   = "index",
  engine    = os.getenv("ENGINE") or "xelatex",
  chapters  = 93,
  chapdir   = "chapters",
  chapfmt   = "ch%02d",          -- stable source IDs, not printed chapter numbers
  extra     = { "frontmatter/preface.tex", "references.bib",
                "appendices/appendix-a.tex", "appendices/appendix-b.tex",
                "appendices/appendix-c.tex", "appendices/appendix-d.tex" },
  fonts     = { "Source Serif 4", "Source Sans 3", "JetBrains Mono" },
  aux_exts  = { "aux", "toc", "lof", "lot", "out", "log", "bbl", "bcf", "blg",
                "run.xml", "idx", "ilg", "ind", "fls", "fdb_latexmk",
                "synctex.gz", "bak" },
}

local is_windows = package.config:sub(1, 1) == "\\"

------------------------------------------------------------------------
-- helpers
------------------------------------------------------------------------

local function quote(s)
  if is_windows then return '"' .. s:gsub('"', '\\"') .. '"' end
  return "'" .. s:gsub("'", "'\\''") .. "'"
end

local function exists(path)
  local f = io.open(path, "r")
  if f then f:close(); return true end
  return false
end

local function run(cmd)
  print("$ " .. cmd)
  local ok, _, code = os.execute(cmd)
  -- Lua 5.1 returns a number, 5.2+ returns true/nil
  if type(ok) == "number" then return ok == 0 end
  return ok == true and (code == 0 or code == nil)
end

local function die(msg)
  io.stderr:write("error: " .. msg .. "\n")
  os.exit(1)
end

local function chapter_path(n)
  return string.format("%s/" .. config.chapfmt, config.chapdir, n)
end

------------------------------------------------------------------------
-- TeX invocations
------------------------------------------------------------------------

-- Run the engine. `includeonly` is an optional list of chapter paths.
local function tex(includeonly)
  local input
  if includeonly and #includeonly > 0 then
    -- \includeonly must appear before \begin{document}; it is legal
    -- before \documentclass, so we can inject it from the command line.
    input = string.format("%s\\includeonly{%s}\\input{%s}",
      "", table.concat(includeonly, ","), config.main)
  else
    input = config.main
  end
  local separator = is_windows and ";" or ":"
  local texinputs = "./support//" .. separator
    .. (os.getenv("TEXINPUTS") or "") .. separator
  local prefix = is_windows
    and ('set "TEXINPUTS=%s" && '):format(texinputs)
    or ("env TEXINPUTS=" .. quote(texinputs) .. " ")
  return run(prefix .. table.concat({
    config.engine,
    "-shell-escape",
    "-interaction=nonstopmode",
    "-halt-on-error",
    "-file-line-error",
    "-jobname=" .. config.jobname,
    quote(input),
  }, " "))
end

local function biber()     return run("biber " .. config.jobname) end
local function makeindex() return run("makeindex " .. config.jobname .. ".idx") end

------------------------------------------------------------------------
-- commands
------------------------------------------------------------------------

local commands = {}

function commands.check()
  local missing = {}
  if not exists(config.main) then missing[#missing + 1] = config.main end
  for n = 1, config.chapters do
    local p = chapter_path(n) .. ".tex"
    if not exists(p) then missing[#missing + 1] = p end
  end
  for _, p in ipairs(config.extra) do
    if not exists(p) then missing[#missing + 1] = p end
  end
  if #missing == 0 then
    print("All source files present.")
    return true
  end
  print(#missing .. " missing file(s):")
  for _, p in ipairs(missing) do print("  " .. p) end
  return false
end

function commands.fonts()
  if is_windows then
    print("Font check is not automated on Windows; verify in the Fonts settings:")
    for _, f in ipairs(config.fonts) do print("  " .. f) end
    return true
  end
  local all_ok = true
  for _, family in ipairs(config.fonts) do
    local p = io.popen("fc-list : family 2>/dev/null")
    local found = false
    if p then
      for line in p:lines() do
        if line:find(family, 1, true) then found = true; break end
      end
      p:close()
    end
    print(string.format("  [%s] %s", found and "ok" or "MISSING", family))
    all_ok = all_ok and found
  end
  return all_ok
end

function commands.quick()
  return tex()
end

function commands.build()
  if not commands.check() then
    die("fix the missing files above (or use `draft` for a partial build)")
  end
  -- Pass 1 creates aux, bcf and idx; then bib and index; then resolve refs.
  local steps = {
    function() return tex() end,
    biber,
    makeindex,
    function() return tex() end,
    function() return tex() end,
  }
  for _, step in ipairs(steps) do
    if not step() then die("build failed") end
  end
  print("\nDone: " .. config.jobname .. ".pdf")
  return true
end

local function selected_chapters(...)
  local nums = { ... }
  if #nums == 0 then die("draft and draftfull require chapter numbers") end
  local list = { "frontmatter/preface" }
  for _, s in ipairs(nums) do
    local n = tonumber(s)
    if not n or n % 1 ~= 0 or n < 1 or n > config.chapters then
      die("invalid chapter number: " .. tostring(s))
    end
    local path = chapter_path(n)
    if not exists(path .. ".tex") then die("missing file: " .. path .. ".tex") end
    list[#list + 1] = path
  end
  return list
end

function commands.draft(...)
  local list = selected_chapters(...)
  -- Two passes are enough for a draft; skip bibliography and index.
  if not tex(list) then die("draft build failed") end
  if not tex(list) then die("draft build failed") end
  print("\nDraft built: " .. config.jobname .. ".pdf")
  return true
end

function commands.draftfull(...)
  local list = selected_chapters(...)
  if not tex(list) then die("partial build failed") end
  if not biber() then die("partial bibliography failed") end
  if not makeindex() then die("partial index failed") end
  if not tex(list) then die("partial build failed") end
  if not tex(list) then die("partial build failed") end
  print("\nPartial build with bibliography and index: " .. config.jobname .. ".pdf")
  return true
end

local function remove(path)
  if exists(path) then os.remove(path) end
end

function commands.clean()
  for _, ext in ipairs(config.aux_exts) do
    remove(config.jobname .. "." .. ext)
  end
  -- Per-chapter .aux files written by \include
  local pattern = is_windows
    and ('dir /b %s\\*.aux 2>nul'):format(config.chapdir)
    or  ('ls %s/*.aux 2>/dev/null'):format(config.chapdir)
  local p = io.popen(pattern)
  if p then
    for line in p:lines() do
      local path = is_windows and (config.chapdir .. "\\" .. line) or line
      remove(path)
    end
    p:close()
  end
  -- Root-level aux files for other front/back matter includes
  for n = 1, config.chapters do remove(chapter_path(n) .. ".aux") end
  remove("frontmatter/preface.aux")
  print("Cleaned auxiliary files.")
  return true
end

function commands.distclean()
  commands.clean()
  remove(config.jobname .. ".pdf")
  print("Removed " .. config.jobname .. ".pdf")
  return true
end

------------------------------------------------------------------------
-- main
------------------------------------------------------------------------

local cmd = arg[1] or "build"
local fn = commands[cmd]
if not fn then
    die("unknown command '" .. cmd .. "'. Try: build, draft, draftfull, quick, fonts, check, clean, distclean")
end

local unpack_args = table.unpack or unpack
local ok = fn(unpack_args(arg, 2))
os.exit(ok and 0 or 1)
