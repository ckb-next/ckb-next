#!/usr/bin/env bash
N=$'\n'
FW="# Do not manually modify this file.${N}# Edit FIRMWARE.unsigned instead, and call ./scripts/sign_firmware.sh from the root directory of the repo${N}#${N}$(cat FIRMWARE.unsigned)"
echo -n "$FW" | gpg2 --clearsign -o FIRMWARE -u dev@ckb-next.org --batch --yes
