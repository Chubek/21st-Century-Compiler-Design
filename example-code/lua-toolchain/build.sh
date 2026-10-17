#!/bin/sh
# Usage: sh build.sh /tmp/opencode/lua-tools [gcc|clang]
set -eu
out=${1:?output directory required}
cc=${2:-cc}
src=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
mkdir -p "$out"
# pkg-config returns compiler flags; deliberate word splitting.
for tool in luaas luald; do
    "$cc" -std=c11 -Wall -Wextra -Wpedantic -Werror ${CFLAGS:-} \
      $(pkg-config --cflags lua5.4) "$src/$tool.c" \
      $(pkg-config --libs lua5.4) -o "$out/$tool"
done
