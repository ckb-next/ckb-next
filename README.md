ckb: Corsair K65/K70/K95 Driver for Linux and OSX
=================================================

**ckb** is an open-source driver for Corsair keyboards. It aims to bring the functionality of their proprietary Corsair Utility Engine software to the Linux and Mac operating systems. This project is currently a work in progress, but several features are already complete and the software has full RGB animation support. More features are coming soon. Testing feedback and bug reports are very much appreciated!

**Disclaimer:** ckb comes with no warranty and is not an official Corsair product. It is licensed under the GNU General Public License (version 2) in the hope that it will be useful for those of us wishing to take advantage of the keyboard's features on non-Windows OSes.

If you use and enjoy this project, I'd greatly appreciate it if you could spare a few dollars for a donation. This is completely voluntary, of course - the project will remain free and open source regardless. `:)`

I accept donations through PayPal: [Click here](https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=DCLHFH9S3KZ8W&lc=US&item_name=ckb&item_number=ckb%20GitHub%20Page&no_note=1&no_shipping=1&currency_code=USD&bn=PP%2dDonationsBF%3abtn_donateCC_LG%2egif%3aNonHosted)

Or through Bitcoin: [1LDHCfyDqAziUPtp3a4BdUaKmwphG1yroQ (click for QR code)](https://i.imgur.com/h3gyLiv.png)

Device Support
--------------

ckb currently supports the following Corsair keyboards:

* K65 RGB
* K70
* K70 RGB
* K95 RGB

I'm interested in adding support for more non-RGB keyboards. Feel free to contact me if you'd like to test them!

When downloading a new version of ckb, please delete your old download first. This helps ensure there are no problems lingering from an old build.

Building for Linux
------------------

Requires Qt5 and libudev (Ubuntu: `qt5-default` and `libudev-dev`. In other distros, Qt5 may be known as `qt5-base` or `libqt5*-devel`). `gcc`, `g++`, and `glibc` are also required. Check with your package manager to make sure you have the correct libraries/headers installed.

You can build the project by running `./qmake-auto && make` in the directory you downloaded it to. The binaries will be placed in a new `bin` directory assuming they compile successfully. To use the driver, first start the daemon program by running `sudo bin/ckb-daemon`. After that, open the `ckb` binary as a normal user to start the user interface.

Troubleshooting (Linux)
-----------------------

If you have problems connecting the keyboard to your system (keyboard doesn't respond, ckb-daemon doesn't recognize it or can't connect the keyboard), try adding the following to your kernel's `cmdline`:

* K65 RGB: `usbhid.quirks=0x1B1C:0x1B17:0x20000000`
* K70: `usbhid.quirks=0x1B1C:0x1B09:0x20000000`
* K70 RGB: `usbhid.quirks=0x1B1C:0x1B13:0x20000000`
* K95 RGB: `usbhid.quirks=0x1B1C:0x1B11:0x20000000`

For instructions on adding `cmdline` parameters in Ubuntu, see https://wiki.ubuntu.com/Kernel/KernelBootParameters

If the keyboard still doesn't work, try replacing `0x20000000` with `0x00000004`. Note that this will cause the kernel driver to ignore the keyboard completely, so you'll need to make sure ckb-daemon is running at boot or else you'll have no keyboard input.

Installing as a Service (Linux / Systemd)
-----------------------------------------

A service file is provided so that Linux users with systemd can launch the daemon on startup. To use it, first copy the binary files and the service to their system directories:

`sudo cp -R bin/* /usr/bin && sudo cp systemd/ckb-daemon.service /usr/lib/systemd/system`

To launch the daemon and enable it at start-up:

`sudo systemctl start ckb-daemon && sudo systemctl enable ckb-daemon`

OSX Binaries
------------

Pre-compiled binaries for OSX are provided by Xiy. The latest OSX binary can be downloaded here: http://mbx.cm/t/_guh4 (version: alpha-v0.0.27)

**Note:** In order to use the OSX binaries you currently need to install Qt5 first: http://www.qt.io/download-open-source/

Make sure to install it to `/Applications/Qt` (the default path is `/Users/<yourusername>/Qt` which won't work). This is a temporary fix; a more permanent solution will be provided in a future version. If the binary version doesn't run, try following the build instructions instead.

To install it, open the dmg and copy "ckb" to your Applications folder. To start the driver, open Terminal and run `sudo /Applications/ckb.app/Contents/Resources/ckb-daemon`. After that, open the ckb app to start the user interface.

Building for OSX
----------------

Install the latest version of Xcode from the App Store. Open Xcode, accept the license agreement, and wait for it to install any additional components (if necessary). When you see the "Welcome to Xcode" screen, the setup is finished and you can close the app. Then install Qt5 from here: http://www.qt.io/download-open-source/

Open ckb.pro in Qt Creator. You should be prompted to configure the project (make sure the "Desktop" configuration is selected and not iOS). Once it's finished loading the project, press `Cmd+B` or select `Build > Build Project "ckb"` from the menu bar. When it'd done, you should see a newly-created `ckb.app` application in the project directory. Exit Qt Creator.

Before launching ckb, the `ckb-daemon` program needs to be run as root. Open the ckb directory in a Terminal window and run `sudo ckb.app/Contents/Resources/ckb-daemon`. Then open `ckb.app` as a normal application to start the user interface.

Troubleshooting (OSX)
---------------------

Make sure your system is up-to-date and that Xcode works on its own. Compile problems can usually be resolved by rebooting your computer and/or reinstalling Qt. Be sure to delete the `ckb` directory as well as any automatically-generated `build-ckb` directories and start fresh with a new download.

If you've rebound your modifier keys in System Preferences, the changes will not be recognized anymore. You can rebind them again within the application.

Usage
-----

See `DAEMON.md` for info about the daemon program.

The user interface is still a work in progress.

**Major features:**
- Control multiple keyboards independently (note: not tested)
- United States and European keyboard layouts
- Customizable key bindings
- Per-key lighting and animation
- Reactive lighting
- Multiple profiles/modes with hardware save function

**Roadmap** (roughly in order)
- **v0.1 release:**
- Ability to share lighting or binding between multiple modes
- Ability to copy animations, settings to different modes
- User interface for firmware updates
- System service files so that ckb-daemon can be run at system start.
- **v0.2 release:**
- More functions for the Win Lock key
- Key combos
- Key macros, other advanced keypress features like running a custom command
- (Daemon) Macros with timing info, delays, repeats
- (Daemon) Notification macros
- (Daemon) Ability to generate mouse events
- **v0.3 release:**
- Ability to store profiles separately from devices, import/export them
- Ability to tie profiles to which application has focus
- Timers
- **v0.4 release:**
- Ability to import CUE profiles
- **v1.0 release:**
- OSD? (Not sure if this can actually be done)
- Extra settings?
- ????

Closing ckb will actually minimize it to the system tray. Use the Quit option from the tray icon or the settings screen to exit the application.

Known issues
------------

- The system tray icon doesn't always appear in Linux. Apparently this is a known Qt bug. To bring ckb back after hiding it, re-run the application.
