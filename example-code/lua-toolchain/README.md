# Lua-assisted x86-64 assembly and linking

These C11 case studies accompany source chapters 53–60. `luaas` embeds Lua 5.4
as a preprocessing language and emits real ELF64 ET_REL objects. `luald` embeds
Lua 5.4 as a memory-map language and emits static ELF64 ET_EXEC images. Neither
tool invokes another assembler or linker to implement its work.

## Build and run

Requires C11, Lua **5.4** development headers/library, pkg-config, and POSIX
`chmod`. Native execution tests require x86-64 Linux; verification additionally
requires GCC, Clang, GNU binutils, and Python 3. The dynamic-loader experiment
uses POSIX dlopen and pthreads. Python is test/model tooling, not the implementation.

```sh
sh example-code/lua-toolchain/build.sh /tmp/opencode/lua-tools gcc
/tmp/opencode/lua-tools/luaas example-code/lua-toolchain/demo.lua /tmp/opencode/lua-tools/demo.o
/tmp/opencode/lua-tools/luaas example-code/lua-toolchain/answer.lua /tmp/opencode/lua-tools/answer.o
/tmp/opencode/lua-tools/luald example-code/lua-toolchain/map.lua /tmp/opencode/lua-tools/demo \
  /tmp/opencode/lua-tools/demo.o /tmp/opencode/lua-tools/answer.o
readelf -h -l /tmp/opencode/lua-tools/demo
/tmp/opencode/lua-tools/demo
```

The demo intentionally exits **42**. `hello.lua` emits a message as numeric byte
directives, uses RIP-relative LEA, writes `Lua maps, ELF runs.` plus newline,
and exits zero. Assemble/link it alone with the same map.

```sh
PYTHONDONTWRITEBYTECODE=1 python3 example-code/lua-toolchain/test.py
PYTHONDONTWRITEBYTECODE=1 python3 example-code/lua-toolchain/checks.py
```

The test driver builds both tools under GCC/Clang, with strict warnings and again
with ASan/UBSan/leak detection, in temporary external directories. It executes
native images, compares 276 encoding forms against GNU as, validates patched
fields independently, checks weak/strong/local resolution, malformed objects,
map errors, and exact signed-PC32 endpoints. It also builds the host dynamic/TLS
experiment with GCC and Clang. No generated files belong in the repository.

## Assembler interface

`luaas [--asm] input output.o`. By default input is Lua source executed once with
standard libraries and an `emit(string)` callback. Each call appends a newline.
Lua completes successfully before assembly starts; a script error produces no
object. With `--asm`, input is already expanded text. The assembly error location
is the **generated** line number, not a claimed Lua expansion-stack location.
Embedded source NUL is rejected; target NUL data is `.byte 0`.

The assembly language is case-sensitive, destination-first, one statement per
line, with optional `label:` and semicolon comments. Names begin with a letter,
underscore, or dot and continue with alphanumeric/underscore/dot characters.
Directives and opcodes require whitespace before their operands. There is no
quoted assembly-string syntax. Labels are private unless declared nonlocal.

Supported directives:

- `.text`, `.rodata`, `.data`, `.bss`: switch independent section counters.
- `.global name`, `.extern name`, `.weak name`: declare one nonlocal name.
  Global and extern both declare strong linkage; either can precede a definition.
  Conflicting weak/strong declarations within one object are rejected.
- `.byte expr`, `.long expr`, `.quad expr`: one 1/4/8-byte field. Symbolic byte
  fields are unsupported. Literal input is signed-64; 1/4-byte fields accept
  `[-128,255]` and `[-2^31,2^32-1]` bit-pattern ranges, respectively.
- `.align n`: power-of-two byte alignment, at most 4096. File-backed padding
  is **zero**, not automatically x86 NOPs; do not fall through such code padding.
- `.zero n`: append zero bytes or extend NOBITS storage; no negative count.

Expressions are one base-0 signed-64 integer (`0x` hex, leading-zero octal,
otherwise decimal), or `symbol`, `symbol+integer`, `symbol-integer`. General
expression trees and symbol differences are discussed but not implemented.
Unknown private names at end of assembly are errors; declare intended externals.

Supported register names: `rax rcx rdx rbx rsp rbp rsi rdi` and their corresponding
32-bit names `eax ecx edx ebx esp ebp esi edi`. Supported instructions:

| Form | Encoding policy |
| --- | --- |
| `mov reg, reg`, `xor reg, reg` | Equal widths, register ModR/M form. |
| `mov reg, expr` | B8+register; full imm64 for 64-bit destination, imm32 for 32-bit. |
| `add/sub/cmp reg, integer` | 81 opcode with signed imm32, no short-immediate selection. |
| `lea reg64, symbol+addend` | RIP-relative address formation only; not general Intel memory syntax. |
| `call/jmp/jz/jnz symbol+addend` | Near rel32, no short-branch relaxation. |
| `ret`, `syscall`, `nop`, `ud2` | No operands. |

All supported symbolic fields become ELF RELA entries; PC32 adds -4 because the
field ends at the instruction boundary. Other emitted types are R_X86_64_64
and R_X86_64_32. Local symbols are serialized before globals/weaks, and fixup
symbol indices are remapped accordingly. Output has four allocated sections,
four RELA sections, symbol/string/name tables, and section zero.

## Linker and Lua-map interface

`luald map.lua output.elf input.o ...`. Inputs are ordered; archives are not read.
The map returns a plain table like `map.lua`, with a nonempty entry name,
region array, and section-rule array. Arrays must have dense consecutive integer
keys starting at one; map tables must not have metatables. Unknown extra record
fields have no effect. Lua is executed with standard libraries as a build script,
not a sandbox; there is no Lua instruction/time quota.

Region fields: unique `name`, integer `origin`, integer `length`, `flags` equal
to `r`, `rx`, or `rw`. Origins and lengths are 4096-byte aligned, origins at least
4096, lengths 1..16 MiB in page multiples, and the exclusive endpoint at most
2^47. Regions cannot overlap. Section rules have exact `name`, existing `region`,
and a supported power-of-two `align`. Duplicate exact-name rules are rejected.

Rules are processed in array order, then inputs in command-line order, then
sections in their table order. Effective alignment is the maximum of input and
rule alignments. Nonempty allocated sections must be mapped explicitly.
Zero-size sections can be mapped but do not advance occupied extents. Input
write/execute requirements must be permitted by the region. Initialized data
must precede any nonempty NOBITS contribution within the same region.

One PT_LOAD per occupied region describes its initialized prefix and memory
extent. Headers are sorted by virtual address even if regions were declared in
another order. Payload file offsets are page-aligned, as are virtual bases.
Output includes a non-executable GNU_STACK header, no section table, no dynamic
loader metadata, no CRT startup, and no debug/unwind preservation. It is an
ET_EXEC for the tested freestanding Linux syscall programs, not a general PIE
or hosted-C executable. The output is chmod'd 0755. Stdout is a placement report.

Supported input profile:

- ELF64 little-endian x86-64 ET_REL; SysV/Linux OSABI, version zero; ordinary
  section counts; one static symbol table; no input program headers.
- Allocated PROGBITS/NOBITS sections with only W/A/X flags. Unknown allocated
  types/flags, including processor-property notes and TLS, are rejected.
- Local/global/weak ordinary symbols (NOTYPE, OBJECT, FUNC, SECTION, FILE),
  default/hidden visibility; ordinary or absolute definitions. Absolute values
  are restricted to at most INT64_MAX. Common/group/extended-index inputs are
  unsupported. Strong definitions override weak; first weak wins ties; duplicate
  strong definitions fail. Undefined weak resolves to zero.
- RELA types 64, PC32, PLT32, 32, 32S for allocated fields. PLT32 binds directly
  in this fully static nonpreemptable profile. Narrow values receive signed or
  unsigned range checks as appropriate; 64-bit relocation uses modulo-2^64
  arithmetic. Overlapping fields and relocation into NOBITS are rejected.
- Nonallocated metadata is not emitted; relocations targeting it are not applied.

For GNU-produced fixtures on this host, use `as --64 -mx86-used-note=no` so the
input matches that profile. Unknown allocated notes are not silently discarded.

## Capacities, complexity, and errors

64 objects, 64 sections per object, 4096 symbols per object/assembler unit,
8192 relocations per RELA input section (8192 total assembler fixups), 16 map
regions, 64 rules, 8192 owned allocations, and 16 MiB per byte buffer/file/region.
The **combined executable file** must also fit its 16 MiB buffer cap.
Buffers and ELF arithmetic are checked before indexing/patching.

Both programs return 0 for successful construction and 2 for diagnosed failure.
They validate semantic input before opening the output file. A final I/O write
failure removes the partial output; output replacement is not atomic. Existing
files remain untouched on pre-write semantic failures. Lua errors can have
non-string values and receive a fallback diagnostic.

This implementation uses process-lifetime allocations released at exit, linear
symbol lookup, and repeated global-definition scans. With T symbols and R
relocations, resolution can cost O(T^2+RT). Placement matching costs O(QH) for
Q rules and H sections, plus copied bytes. The design prioritizes auditability
within its explicit capacities; it is not a throughput benchmark.

`checks.py` implements independent small models of the chapter's relaxation,
archive extraction, and section reachability examples. Those models do not imply
the C tools implement relaxation, archives, section GC, LTO, dynamic linking,
or TLS. `dynamic-main.c`/`dynamic-lib.c` exercise the **host** loader separately.
`SOLUTIONS.md` gives the forty checked exercise answers.
