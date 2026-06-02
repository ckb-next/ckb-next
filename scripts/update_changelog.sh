#!/usr/bin/env bash

if [[ $# -ne 3 ]]; then
    echo "Usage $0 0 5 0"
    exit 2
fi

LOG=$(cat CHANGELOG.md | tail -n +2)
DATE=$(date +%F)

PREVPATCH=$(($3-1))
PREVMIN=$2
if [[ $PREVPATCH -lt 0 ]]; then
    PREVMIN=$(($2-1))
    PREVPATCH="FIXME"
    echo "Warning: Previous version needs to be manually modified in [Full Changelog]"
fi
N=$'\n'
NEW="# Change Log${N}${N}## [v$1.$2.$3](https://github.com/ckb-next/ckb-next/tree/v$1.$2.$3) ($DATE)${N}[Full Changelog](https://github.com/ckb-next/ckb-next/compare/v$1.$PREVMIN.$PREVPATCH...v$1.$2.$3)${N}${N}Support for new devices:${N}- ${N}${N}Important bugfixes:${N}- ${N}${N}New features:${N}- ${N}${N}Notes for packagers:${N}- ${N}"
echo "${NEW}${LOG}" > CHANGELOG.md
