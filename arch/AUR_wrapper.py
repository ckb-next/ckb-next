#!/usr/bin/env python3
# Copyright (C) 2022 Tasos Sahanidis

# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

import os
import re
import sys
import subprocess
try:
    from PyQt6.QtWidgets import QApplication, QMessageBox
    from PyQt6.QtCore import Qt
except ModuleNotFoundError:
    from PyQt5.QtWidgets import QApplication, QMessageBox
    from PyQt5.QtCore import Qt

# We need at least argv[0]
if not sys.argv:
    print("argv is empty")
    sys.exit(2)

pgmname = os.path.basename(sys.argv[0])
binpath = sys.argv[0] + ".real"

# Check to see if we can run the real binary
lddout = ""
try:
    subprocess.check_output([binpath, "-v"], stderr=subprocess.STDOUT, text=True)
except Exception as e:
    try:
        ldd = subprocess.run(["ldd", binpath], stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
        lddout = re.sub(r"^\t", "", ldd.stdout, flags=re.M)
    except Exception as e2:
        lddout = str(e2)

    # Initialise the UI
    app = QApplication(sys.argv)
    msg = QMessageBox()
    msg.setIcon(QMessageBox.Icon.Critical)
    msg.setTextFormat(Qt.TextFormat.RichText)
    msg.setWindowTitle(f"Could not launch {pgmname}")
    msg.setText(f"Could not launch {pgmname}.")
    msg.setStandardButtons(QMessageBox.StandardButton.Ok)

    helpurl = "https://github.com/ckb-next/ckb-next/wiki/Linux-Installation#rebuilding-aur-packages"
    suggestion = f'<a href="{helpurl}">Rebuilding {pgmname}</a> and reinstalling from the AUR might correct this.'    
    informativesuffix = "<br><br>Please report this <b>only if</b> rebuilding the package has not resolved this issue."

    if isinstance(e, subprocess.CalledProcessError):
        # We can't be sure that this string won't change in the future,
        # so we have to provide suggestions as a fallback
        if "error while loading shared libraries" in e.output:
            msg.setInformativeText(f'You must <a href="{helpurl}">rebuild {pgmname}</a>.<br><br>This happened because AUR packages are not rebuilt automatically when the system updates.{informativesuffix}')
        else:
            msg.setInformativeText(f'This might have happened because AUR packages are not rebuilt automatically when the system updates.<br><br>{suggestion}{informativesuffix}')
        msg.setDetailedText(f"{e.output}\nReturned: {e.returncode}\n{lddout}")
        msg.exec()
    else:
        msg.setInformativeText(f"{suggestion}{informativesuffix}")
        msg.setDetailedText(f"Exception: {str(e)}\n{lddout}")
        msg.exec()
        # There's no point in running the binary if we got a different exception
        sys.exit(1)
    
# Launch the real binary
os.execv(binpath, sys.argv)
