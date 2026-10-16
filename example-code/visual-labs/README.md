# Visual laboratories (C11)

The book includes these files directly with `inputminted`:

| Chapter | File | Contract illustrated |
| --- | --- | --- |
| 3 | `decimal.c` | Scan digits, stop before the next token, detect unsigned overflow. |
| 28 | `copy.c` | Read all parallel-copy sources before changing destinations. |
| 31 | `depth.c` | Check pop count and capacity before publishing the next depth. |
| 32 | `layout.c` | Round a byte offset to a nonzero alignment without overflow. |

These are small components, not a complete lexer, register allocator, bytecode
verifier, or object-layout implementation. Their interfaces and pointer/aliasing
preconditions are in `visual_labs.h`. No external dependencies are required.
All failure paths leave the caller's output objects unchanged.

Run from the repository root, with assertions enabled:

```sh
cc -std=c11 -Wall -Wextra -Wpedantic -Werror \
  example-code/visual-labs/decimal.c example-code/visual-labs/copy.c \
  example-code/visual-labs/depth.c example-code/visual-labs/layout.c \
  example-code/visual-labs/test_visual_labs.c -o /tmp/opencode/visual-labs
/tmp/opencode/visual-labs
```

Repeat with GCC and Clang; add `-fsanitize=address,undefined` for instrumented
runs. Tests cover decimal boundaries including `UINT_MAX`, all 65,536 byte-valued
swaps, a three-cycle with a repeated source, invalid-copy rollback, 9,000 small
stack transitions plus `SIZE_MAX` boundaries, and 8,192 offset/alignment pairs
plus overflow and zero-alignment rejection. The layout oracle advances one byte
at a time rather than reusing the implementation's rounding formula.
