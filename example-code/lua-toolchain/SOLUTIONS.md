# Checked exercise answers: source chapters 53–60

These are original worked answers. `checks.py` recomputes layout, relaxation,
archive, and reachability examples; `test.py` checks executable tool behavior.

## 53. Assembly language and sections

1. Data was at 8. Aligning to 16 adds 8 bytes; tail is data+16, final data=20,
   text=10. Switching sections never resets the existing counter.
2. Same text base: absolute 31, provided intervening layout is final. Different
   bases: B_text-B_data+31, unsupported by the simple single-base domain.
3. Null=0, local a=1, local b=2, global g=3. Internal g identity maps to ELF
   index 3; a fixup must use that map rather than the original insertion index.
4. Payload=0; memory extent=44; the label after alignment is section+32.
5. Reserve the four-byte displacement inside the known five-byte call and keep
   its field offset, symbol identity, addend, and relocation kind. A second scan
   of the original source text is unnecessary for fixed-size encoding.

## 54. Encoding and objects

1. ModR/M = 0xC0 + (3<<3) + 2 = 0xDA; bytes for mov edx,ebx are 89 DA.
2. P=0x501011, A=-4, displacement=0x6B=107, bytes 6B 00 00 00.
   The processor adds it to 0x501015 and reaches 0x501080.
3. 0x80000000 fits unsigned32 but not a positive signed32 value. Sign-extending
   those low bytes produces 0xFFFFFFFF80000000, not 0x0000000080000000.
4. Section index 9 selects the symbol table; section index 1 selects the patched
   payload; symbol index 6 selects a record within that symbol table.
5. Accepted endpoints -2147483648 and 2147483647; adjacent rejected values
   -2147483649 and 2147483648. Short positive calls do not test either sign/range edge.

## 55. Preprocessing and layout

1. With initial serial zero: .loop_1 and .loop_2, each referenced by its own jnz.
   A shared fresh-name state for the assembly unit, plus a reserved prefix,
   prevents reuse. Runtime counts are loaded as 3 and 5, respectively.
2. `.byte 65`, `.byte 0`, `.byte 10`. Source transport contains no NUL; target
   data does. Lua source may represent the string using `"A\0\n"`.
3. Rows are (A,B,T,U,dA,dB)=(2,2,128,136,126,132),
   (2,5,131,139,129,132), (5,5,134,142,129,132). Widening A shifts both its
   next-instruction address and T by three, leaving their difference unchanged.
4. Padding: 3,2,0 bytes; both 29 and 30 round to 32. The one-byte instruction
   growth is absorbed by one less padding byte.
5. At most 20 changing rounds plus a stable round. A coarse bound is 21*200=4200
   fragment visits for one layout/check sweep per round, up to its constant work.

## 56. Assembler case study

1. Conditional branch starts at 11 and has two opcode bytes: field at 13.
   Call starts at 17 and has one opcode byte: field at 18.
2. Only the immediate of the initial mov ecx changes from 03 00 00 00 to
   07 00 00 00. Its length is unchanged; every symbol offset stays fixed.
3. Pointer: payload 8, memory 8, one absolute64 relocation. Scratch: payload 0,
   memory 32 (plus any placement padding outside its input contribution), no
   initialization relocation; references *to* scratch elsewhere could be relocated.
4. Streaming assembly can still be correct if all results remain unpublished,
   but it mixes assembler state and Lua failure cleanup. Emitting output files
   incrementally would require rollback. Completing preprocessing first provides
   a simpler explicit phase and failure boundary.
5. Production assemblers may choose a shorter valid sign-extending form for a
   small 64-bit immediate. movabs requests the same full-width form as luaas,
   so byte equality tests the encoder rather than an unrelated selection policy.

## 57. Resolution and archives

1. Local identities are (alpha.o, local symbol index) and (beta.o, local symbol
   index); global helper participates in the global name table and selects the
   second object's defining entry. Equal spellings alone do not merge locals.
2. The strong definition wins in both orders. Neither sequence has two strong
   definitions, so neither is an error. Weak-only ties use first object order.
3. B initially extracts nothing; A extracts draw and introduces blend. Without
   rescan, blend remains unresolved. In a group the next B visit selects blend,
   then the A visit selects helper; all demands are satisfied.
4. An archive member enters only when extraction policy selects it. If f is
   already satisfied and no other symbol selects the member, its strong f never
   participates in the link and cannot override the existing weak definition.
5. At most 12 changing sweeps and one stable sweep: 13*100=1300 summary operations
   under the stated full-sweep cost, plus incorporation/resolution work.

## 58. Layout and relocation

1. Second start is code+48 (0x401030 at the original base); total code extent 54.
2. BSS begins at data+32; memory extent=64. With eight file bytes, 56 bytes are
   supplied as zero tail, including 24 alignment bytes and 32 BSS bytes.
3. Loop and call PC32 displacements remain -12 and 10. The absolute pointer
   changes from 0x401020 to 0x601020; it is stored in the separate data segment.
4. The program header maps file offset 0x2000 to virtual address 0x406000.
   These coordinates serve file access and runtime addressing, respectively.
5. The fifth byte (zero-based offset 4) is read at 0x100084 and written at
   0x200014. A program pointer contains the run address 0x200014.

## 59. Dynamic linking and transformations

1. Old difference=0x80; new place=0x401200, target=0x401280, difference still
   0x80. An absolute pointer changes to 0x401280.
2. Offset 40 is hex 0x28: addresses 0x710028 and 0x720028. The current thread
   selects the storage base; one process-wide absolute address lacks that coordinate.
3. Live={entry,a,b}; the a/b cycle terminates under visited marking. Orphan stays
   dead because its outgoing edge does not make the graph traversable backward.
4. Equal placeholder bytes hide different deferred targets. Applying the distinct
   relocations can produce different call behavior; folding needs relocation-aware
   equivalence and an address-significance policy as well.
5. The function pointer can point into an unmapped library after unload; the
   worker may also still access that module's TLS. Keep the handle/lifetime valid
   until all executing users have finished, here by joining before dlclose.

## 60. Linker case study

1. Rules and command-line inputs still determine input placement. PT_LOAD headers
   are emitted in ascending virtual address, code before data in the original map,
   using a permutation that preserves stored region IDs.
2. Round 4090 up to 4096. Available capacity is then zero, so eight bytes do not
   fit. The guard n <= capacity-offset fails without an overflowing end calculation.
3. A one-past-end label can describe a boundary or size computation. It does not
   identify an instruction byte inside that executable contribution, so it fails
   the entry-point containment check.
4. Offsets 0 through 4 inclusive. Check offset <= 12 and width <= 12-offset;
   offset 5 would need bytes beyond the initialized payload.
5. Recompute affected input placements and symbol addresses, call displacement,
   absolute pointer, entry if moved, program-header extents/file positions, and
   map report. The local loop displacement may remain invariant if its entire
   interval moves together, but must be justified or recomputed rather than assumed.
