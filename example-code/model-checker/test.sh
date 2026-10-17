#!/bin/sh
# Chapter 48: strict C11, two compilers, normal and ASan/UBSan runs.
set -eu
src=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
out=${OUT_DIR:-/tmp/opencode/model-checker}
mkdir -p "$out"
export TMPDIR="$out"
export ASAN_OPTIONS=detect_leaks=1:halt_on_error=1
export UBSAN_OPTIONS=halt_on_error=1
for cc in gcc clang; do
    "$cc" --version | head -n 1
    for mode in strict sanitize; do
        flags="-O2"
        if [ "$mode" = sanitize ]; then
            flags="-O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer"
        fi
        # Intentional word splitting of the fixed compiler-option list.
        "$cc" -std=c11 -Wall -Wextra -Wpedantic -Werror $flags \
            "$src/checker.c" "$src/test_checker.c" -o "$out/test-$cc-$mode"
        "$cc" -std=c11 -Wall -Wextra -Wpedantic -Werror $flags \
            "$src/checker.c" "$src/main.c" -o "$out/checker-$cc-$mode"
        echo "$cc $mode"
        ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
        UBSAN_OPTIONS=halt_on_error=1 "$out/test-$cc-$mode" "$src"
        bin="$out/checker-$cc-$mode"
        set +e
        "$bin" "$src/stale.graph" > "$out/stale-$cc-$mode.txt"
        status=$?
        set -e
        test "$status" -eq 1
        grep -qx 'trace: 0 1 2 3' "$out/stale-$cc-$mode.txt"
        "$bin" "$src/fixed.graph" > "$out/fixed-$cc-$mode.txt"
        grep -qx safe "$out/fixed-$cc-$mode.txt"
        set +e
        "$bin" "$src/fixed.graph" 0 > "$out/limit-$cc-$mode.txt"
        status=$?
        set -e
        test "$status" -eq 3
        grep -qx resource-limit "$out/limit-$cc-$mode.txt"
        for budget in -1 +1 184467440737095516160 '' 1x; do
            set +e
            "$bin" "$src/fixed.graph" "$budget" > "$out/invalid-$cc-$mode.txt"
            status=$?
            set -e
            test "$status" -eq 2
        done
    done
done
echo 'four build/test configurations and CLI exit checks passed'
