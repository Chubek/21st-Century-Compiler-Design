#!/usr/bin/env texlua
-- Run from the repository root: texlua book/build.lua [check|build]
-- Uses the shared class, style, fonts, and bibliography. Trusted input only:
-- minted requires shell escape. ENGINE may be xelatex or lualatex.
local function exists(path)
  local f = io.open(path, "r")
  if not f then return false end
  f:close()
  return true
end

local function check()
  local files = {
    "book/pamphlet.ltx", "references.bib",
    "support/21st-century-cc-book.cls", "support/compiler-pseudocode.sty",
    "example-code/pebbleir/pebbleir.h", "example-code/pebbleir/pebbleir.c",
    "example-code/pebbleir/demo.c", "example-code/pebbleir/test_pebbleir.c",
  }
  for n = 1, 16 do
    files[#files + 1] = string.format("book/chapters/ch%02d.tex", n)
  end
  local ok = true
  for _, path in ipairs(files) do
    if not exists(path) then
      io.stderr:write("Missing: " .. path .. "\n")
      ok = false
    end
  end
  if ok then print("All pamphlet source files present (16 chapters).") end
  return ok
end

local function quote(s) return "'" .. s:gsub("'", "'\\''") .. "'" end
local function run(cmd)
  print("$ " .. cmd)
  local ok, _, code = os.execute(cmd)
  if type(ok) == "number" then ok = ok == 0 else ok = ok and code == 0 end
  if not ok then
    io.stderr:write("Pamphlet build failed; inspect book/pamphlet.log.\n")
    os.exit(1)
  end
end

local command = arg[1] or "build"
if command ~= "check" and command ~= "build" then
  io.stderr:write("Usage from repository root: texlua book/build.lua [check|build]\n")
  os.exit(1)
end
if not check() then os.exit(1) end
if command == "check" then os.exit(0) end
local engine = os.getenv("ENGINE") or "xelatex"
if engine ~= "xelatex" and engine ~= "lualatex" then
  io.stderr:write("ENGINE must be xelatex or lualatex.\n")
  os.exit(1)
end
local inputs = "./support//:" .. (os.getenv("TEXINPUTS") or "") .. ":"
local tex = "env TEXINPUTS=" .. quote(inputs) .. " " .. engine
  .. " -shell-escape -interaction=nonstopmode -halt-on-error -file-line-error"
  .. " -output-directory=book -jobname=pamphlet book/pamphlet.ltx"
run(tex)
run("biber --input-directory book --output-directory book pamphlet")
run("makeindex -o book/pamphlet.ind -t book/pamphlet.ilg book/pamphlet.idx")
run(tex)
run(tex)
print("Done: book/pamphlet.pdf")
