#!/usr/bin/env python3
import sys
import re

defs = "../src/daemon/protocol.h"
defs_file = open(defs, "r")
defs_list = defs_file.readlines()

cmds = []
for cmd in defs_list:
    match = re.match("#define[\s]+([\S]+)[\s]+(0x[\da-f]+|\d+)", cmd)
    if match:
        cmds.append((match.group(1), match.group(2)))
#print(str(cmds))
defs_file.close()

stdin = sys.stdin.readlines()
print()
for stdline in stdin:
    finalstr = stdline
    for cmd in cmds:
        finalstr = finalstr.replace(cmd[0], cmd[1])
    print(finalstr, end = "")
