#!/usr/bin/env python3
import sys
import re
import os

defs = "../src/daemon/protocol.h"
defspath = os.path.dirname(__file__) + "/" + defs
defs_file = open(defspath, "r")
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
