ckb: Corsair K70/K95 RGB Driver for Linux and OSX
=================================================

ckb is a userspace (non-kernel) driver for Corsair RGB keyboards for Linux and OSX. It contains a daemon program, which runs in the background and must be started as root, as well as a user interface which may be run by any user.

**Note:** When downloading a new version of ckb, please delete your old download first and build it again from scratch. This helps ensure there are no problems lingering from an old build.

Linux instructions
------------------

`qt5-base`, `libudev`, `gcc`, `g++`, and `glibc` are required. Check with your package manager to make sure you have the correct libraries/headers installed (Note: on Ubuntu you need `qt5-default` and `libudev-dev`). You also need a kernel with uinput support (all distros should ship with this by default; don't worry about it unless you're running a custom kernel).

You can build the project by running `qmake && make` in the directory you downloaded it to. The binaries will be placed in a new `bin` directory assuming they compile successfully. To start the daemon, run `sudo bin/ckb-daemon`. After that, open the `ckb` binary as a normal user to start the user interface.

**Note:** If you have problems connecting the keyboard to your system, try adding the following to your kernel's cmdline (for K70):

`usbhid.quirks=0x1B1C:0x1B13:0x20000000`

If you own a K95, replace `1B13` with `1B11`. For instructions on adding cmdline parameters in Ubuntu, see here: https://wiki.ubuntu.com/Kernel/KernelBootParameters

OSX instructions
----------------

Install the latest version of Xcode from the App Store. Then install Qt5 from here: http://www.qt.io/download-open-source/

Open ckb.pro in Qt Creator. You should be prompted to configure the project (the default settings should work). Once it's finished loading the project, press `Cmd+B` or select `Build > Build Project "ckb"` from the menu bar. The binaries should be placed in a newly-created `bin` directory assuming they compile successfully. Exit Qt Creator.

**Note:** The project is intended to compile on OSX Yosemite. If you're running an earlier version, open the `.pro` files and find the `QMAKE_MAC_SDK` lines. Edit them from `macosx10.10` to your OSX version, e.g. `macosx10.9` for Mavericks.

`ckb-daemon` needs to be run as root. Open the `bin` directory in a Terminal window and run `sudo ./ckb-daemon`. Then open `ckb.app` as a normal application to start the user interface.

If you've rebound your modifier keys in System Preferences, those changes won't work anymore. You have to switch them using ckb-daemon's `bind` command. For instance, to switch Cmd and Ctrl, run: `echo bind lctrl:lwin lwin:lctrl rctrl:rwin rwin:rctrl > /tmp/ckb1/cmd`. This functionality is coming to the user interface soon.

Usage
-----

See `DAEMON.md` for info about the daemon program.

The user interface is still a work in progress.

**Major features:**
- Control multiple keyboards independently (note: not tested)
- United States and European keyboard layouts
- Per-key lighting and animation
- Reactive lighting
- Multiple profiles/modes with hardware save function

**Roadmap** (roughly in order)
- **v0.1 release:**
- (Daemon) Allow the daemon to disconnect all keyboards without shutting down, reconnect later. This way ckb can soft stop/soft start the daemon, because using the daemon without ckb running isn't very useful.
- (Daemon) Indicator (Num Lock, etc) notifications
- System service files so that ckb-daemon can be run at system start.
- **v0.2 release:**
- Key rebinding in ckb
- More functions for the Win Lock key
- **v0.3 release:**
- (Daemon) Repeatable key macros, notification macros
- (Daemon) Ability to generate mouse press/release events
- Key combos
- Key macros, other advanced keypress features like running a custom command
- **v0.4 release:**
- Ability to tie profiles to which application has focus, or switch them with keys
- Timers
- **v1.0 release:**
- OSD? (Not sure if this can actually be done)
- Extra settings?
- ????

Closing ckb will actually minimize it to the system tray. Use the Exit option from the tray icon or the settings screen to exit the application.

Known issues
------------

- The system tray icon doesn't always appear in Linux. Apparently this is a known Qt bug. To bring ckb back after hiding it, re-run the application.
