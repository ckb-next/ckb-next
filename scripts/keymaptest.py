#!/usr/bin/env python3
import re
import os
import signal
import subprocess
import time

keymaps = []
with open("src/daemon/usb.c", "r") as f:
    for l in f:
        if m := re.search(r'        return "(.*)";', l):
            mg = m.group(1)
            if mg != "corsair":
                keymaps.append(mg)

ckb = "./build/bin/ckb-next"
daemon = "/dev/input/ckb1/features"
subprocess.run([ckb, "-c"])

for km in keymaps:
    print("Now testing " + km)
    f = open(daemon, "r")
    features = f.read().split()
    f.close()
    
    features[1] = km

    while True:
        try:
            f = open(daemon, "w")
            break
        except OSError:
            input(f"Error opening {daemon} for writing. Press enter to try again")
    f.write(" ".join(features))
    f.close()

    subprocess.run([ckb])
    print("Quit ckb-next to continue")

print("Finished!")
