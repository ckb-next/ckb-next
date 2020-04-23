#!/usr/bin/env bash

if [[ ! -d "src/daemon" ]]; then
    echo "This script must be ran at the root of the repository"
    echo "./scripts/clang-format.sh"
    exit
fi

find src/animations src/daemon src/gui src/libs/ckb-next -iname "*.h" -o -iname "*.cpp" -o -iname "*.c" | xargs clang-format-11 -i
