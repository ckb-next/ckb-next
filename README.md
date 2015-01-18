ckb: Corsair K65/K70/K95 Driver for Linux and OSX
=================================================

**ckb** is an open-source driver for Corsair RGB keyboards. It aims to bring the functionality of their proprietary Corsair Utility Engine software to the Linux and Mac operating systems. This project is currently a work in progress, but several features are already complete and the software has full RGB animation support. More features are coming soon. Testing feedback and bug reports are very much appreciated!

*ckb currently does not support non-RGB Corsair keyboards, but if you have one and are willing to do some tests, don't hesitate to contact me. I'm intrested in adding support for them.*

**Disclaimer:** ckb comes with no warranty and is not an official Corsair product. It is licensed under the GNU General Public License (version 2) in the hope that it will be useful for those of us wishing to take advantage of the keyboard's features on non-Windows OSes.

When downloading a new version of ckb, please delete your old download first. This helps ensure there are no problems lingering from an old build.

If you use and enjoy this project, I'd greatly appreciate it if you could spare a few dollars for a donation. This is completely voluntary, of course - the project will remain free and open source regardless. `:)`

I accept donations through PayPal: [Click here](https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=DCLHFH9S3KZ8W&lc=US&item_name=ckb&item_number=ckb%20GitHub%20Page&no_note=1&no_shipping=1&currency_code=USD&bn=PP%2dDonationsBF%3abtn_donateCC_LG%2egif%3aNonHosted)

Or through Bitcoin: [1LDHCfyDqAziUPtp3a4BdUaKmwphG1yroQ (click for QR code)](https://i.imgur.com/h3gyLiv.png)

Building for Linux
------------------

Requires Qt5 and libudev (Ubuntu: `qt5-default` and `libudev-dev`. Qt5 packages may also be under `qt5-base` or `libqt5*-devel`). `gcc`, `g++`, and `glibc` are also required. Check with your package manager to make sure you have the correct libraries/headers installed.

You can build the project by running `qmake-qt5 && make` in the directory you downloaded it to. If you don't have `qmake-qt5` (e.g. Ubuntu), run `qmake && make` instead. The binaries will be placed in a new `bin` directory assuming they compile successfully. To use the driver, first start the daemon program by running `sudo bin/ckb-daemon`. After that, open the `ckb` binary as a normal user to start the user interface.

**Note:** If you have problems connecting the keyboard to your system, try adding the following to your kernel's `cmdline`:

* K65: `usbhid.quirks=0x1B1C:0x1B17:0x20000000`
* K70: `usbhid.quirks=0x1B1C:0x1B13:0x20000000`
* K95: `usbhid.quirks=0x1B1C:0x1B11:0x20000000`

For instructions on adding `cmdline` parameters in Ubuntu, see https://wiki.ubuntu.com/Kernel/KernelBootParameters

OSX Binaries
------------

Pre-compiled binaries for OSX are provided by Xiy. The latest OSX binary can be downloaded here: http://mbx.cm/t/_guh4 (version: alpha-v0.0.27)

To install it, open the dmg and copy "ckb" to your Applications folder. To start the driver, open Terminal and run `sudo /Applications/ckb.app/Contents/Resources/ckb-daemon`. After that, open the ckb app to start the user interface.

Building for OSX
----------------

Install the latest version of Xcode from the App Store. Then install Qt5 from here: http://www.qt.io/download-open-source/

Open ckb.pro in Qt Creator. You should be prompted to configure the project (make sure the "Desktop" configuration is selected and not iOS). Once it's finished loading the project, press `Cmd+B` or select `Build > Build Project "ckb"` from the menu bar. When it'd done, you should see a newly-created `ckb.app` application in the project directory. Exit Qt Creator.

Before launching ckb, the `ckb-daemon` program needs to be run as root. Open the ckb directory in a Terminal window and run `sudo ckb.app/Contents/Resources/ckb-daemon`. Then open `ckb.app` as a normal application to start the user interface.

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
- Key rebinding in ckb
- (Daemon) Allow the daemon to disconnect all keyboards without shutting down, reconnect later. This way ckb can soft stop/soft start the daemon, because using the daemon without ckb running isn't very useful.
- System service files so that ckb-daemon can be run at system start.
- **v0.2 release:**
- (Daemon) Repeatable key macros, notification macros
- (Daemon) Ability to generate mouse press/release events
- More functions for the Win Lock key
- Key combos
- Key macros, other advanced keypress features like running a custom command
- **v0.3 release:**
- Ability to tie profiles to which application has focus, or switch them with keys
- Timers
- **v1.0 release:**
- OSD? (Not sure if this can actually be done)
- Extra settings?
- ????

Closing ckb will actually minimize it to the system tray. Use the Quit option from the tray icon or the settings screen to exit the application.

Known issues
------------

- The system tray icon doesn't always appear in Linux. Apparently this is a known Qt bug. To bring ckb back after hiding it, re-run the application.
