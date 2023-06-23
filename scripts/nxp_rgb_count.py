#!/usr/bin/env python3
import sys

# Reads an NXP (single colour) frame from stdin and returns the index of the first led that's fully on
# Args are indices to ignore (for buttons that can not be turned off)
ignore = set([int(x) for x in sys.argv[1:]])
cmdi = 0
leds = []
while cmdi < 3:
    expected = f"7f 0{cmdi + 1} "
    cmd = input().lower().strip()
    if not cmd.startswith(expected):
        print("Try again. Expecting " + expected)
        continue

    cmdi += 1

    cmdl = cmd.split(" ")
    leds += cmdl[4:]

for i in range(len(leds)):
    if len(leds[i]) != 2:
        print(f"Invalid data at {i}")
    elif leds[i] == "ff":
        if i in ignore:
            continue
        print(f"LED found at {i} {hex(i)}")
        break
