ckb: Corsair K70/K95 Driver for Linux and OSX
=============================================

ckb is a userspace (i.e. non-kernel) driver for Corsair RGB keyboards for Linux and OSX. It contains a daemon program, which runs in the background and must be started as root, as well as a user interface which may be run as any user.

Building for Linux
------------------

`qt5-base`, `libudev`, `gcc`, `g++`, and `glibc` are required. Check with your package manager to make sure you have the correct libraries/headers installed (Note: on Ubuntu you need `qt5-default` and `libudev-dev`). You also need a kernel with uinput support (all distros should ship with this by default; don't worry about it unless you're running a custom kernel).

You can build the project by running `qmake && make` in the directory you downloaded it to. The binaries will be placed in a new `bin` directory assuming they compile successfully. To start the daemon, run `sudo bin/ckb-daemon`. After that, open the `ckb` binary as a normal user to start the user interface.

Building for OSX
----------------

Install Qt5 from here: http://www.qt.io/download-open-source/ (you'll also be prompted to install Xcode from the app store if you don't have it already)

Open ckb.pro in Qt Creator. You should be prompted to configure the project (the default settings should work). Open `Build` from the menu bar and select `Run qmake`. Now press Cmd+B to build the project. The binaries should be placed in a newly-created `bin` directory assuming they compile successfully. Exit Qt Creator.

**Note:** The project is intended to compile on OSX Yosemite. If you're running an earlier version, open `ckb.pro`, `src/ckb/ckb.pro`, and `src/ckb-daemon/ckb-daemon.pro` and find the `QMAKE_MAC_SDK` lines. Edit them from `macosx10.10` to your OSX version, e.g. `macosx10.9` for Mountain Lion.

ckb-daemon needs to be run as root. Open the `bin` directory in a Terminal window and run `sudo ./ckb-daemon`. Then open `ckb.app` as a normal application to start the user interface.

If you've rebound your modifier keys in System Preferences, those changes won't work anymore. You have to switch them using ckb-daemon's `bind` command. For instance, to switch Cmd and Ctrl, run: `echo bind lctrl:lwin lwin:lctrl rctrl:rwin rwin:rctrl > /tmp/ckb1/cmd`. This functionality is coming to the user interface soon.

Usage
-----

See `DAEMON.md` for info about the daemon program.

The user interface is currently very limited and only supports a handful of options. They should be self-explanatory.

**Coming soon:**
- More animations
- Reactive lighting for keypresses
- Key rebinding
- Key macros

Closing ckb will actually minimize it to the system tray. Use the Exit option from the tray icon or program settings to exit the application.

Known issues
------------

- The system tray icon doesn't always appear in Linux. Apparently this is a known Qt bug.
- The Linux driver doesn't work 100% consistently and can be very slow to start up or shut down. This seems to be a driver/firmware conflict at an OS level, so it probably can't be fixed in ckb.
