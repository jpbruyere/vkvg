#!/bin/bash

find . -iname "*.h" -o -iname "*.c" -o -iname "*.cpp" -o -iname "*.hpp" -not -path "build/*" \
    | xargs clang-format -i -fallback-style=none

exit 0
