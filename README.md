ckb: Corsair K65/K70/K95 Driver for Linux and OSX
=================================================

**ckb** is an open-source driver for Corsair keyboards. It aims to bring the features of their proprietary software to the Linux and Mac operating systems. This project is currently a work in progress but already supports much of the same functionality, including full RGB animations. More features are coming soon. Testing and bug reports are appreciated!

**Disclaimer:** ckb is not an official Corsair product. It is licensed under the GNU General Public License (version 2) in the hope that it will be useful, but with NO warranty of any kind.

If you use and enjoy this project, I'd appreciate if you could spare a few dollars for a donation. This is completely voluntary - the project will remain free and open source regardless. `:)`

I accept donations through PayPal: [Click here](https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=DCLHFH9S3KZ8W&lc=US&item_name=ckb&item_number=ckb%20GitHub%20Page&no_note=1&no_shipping=1&currency_code=USD&bn=PP%2dDonationsBF%3abtn_donateCC_LG%2egif%3aNonHosted)

Or through Bitcoin: [1LDHCfyDqAziUPtp3a4BdUaKmwphG1yroQ (click for QR code)](https://i.imgur.com/h3gyLiv.png)

Device Support
--------------

ckb currently supports the following Corsair keyboards:

* K65 RGB
* K70
* K70 RGB
* K95*
* K95 RGB

\* = hardware playback not yet supported. Settings will be saved to software only.

When downloading a new version of ckb, please delete your old download first. This helps ensure there are no problems lingering from an old build. See the Troubleshooting section if you have any problems.

Linux Installation
------------------

Requires Qt5, libudev, zlib, gcc, g++, and glibc (Ubuntu: `qt5-default`, `libudev-dev`, `build-essential`, `zlib1g-dev`. In other distros, Qt5 may be known as `qt5-base` or `libqt5*-devel`). Check with your package manager to make sure you have the correct libraries/headers installed.

You can download ckb using the "Download zip" option on the right. Extract it and open the ckb-master directory. The easiest way to install ckb is to double-click the `quickinstall` script and run it in a Terminal. It will attempt to build the application, and if all goes well, will ask if you'd like to install/run ckb on your system (press enter to proceed; default answer is "yes"). If the build doesn't work for some reason, or if you'd like to build ckb yourself, see `BUILD.md` for instructions.

OSX Installation
----------------

#### Binary download:

Pre-compiled binaries for OSX are provided by Xiy. They may be updated less frequently.

[Click here to download the latest version (alpha-v0.0.52).](https://github.com/ccMSC/ckb/releases/download/0.0.52/ckb-0.0.52-osx.zip)

The easiest way to install the driver is with the `quickinstall` script. To get it, first click "Download zip" on the right side of the screen to download the source code. Copy the `quickinstall` script and the `service` folder out of the ckb-master folder, and place them in your downloads next to ckb.app _(Note: make sure you copy the scripts OUT of the directory, do not move ckb.app IN to the source directory. Otherwise the script will try to compile the source)._ Double-click on `quickinstall` and it will offer to install/run the app for you. The default answer to all of the questions is "yes"; press enter to proceed. You can close the terminal window after it finishes.

#### Building from source:

Install the latest version of Xcode from the App Store. Open Xcode, accept the license agreement, and wait for it to install any additional components (if necessary). When you see the "Welcome to Xcode" screen, the setup is finished and you can close the app. Then install Qt5 from here: http://www.qt.io/download-open-source/

Like the binary version, it's easiest to install ckb with `quickinstall`. You do not need to copy or move any files since you already have the source downloaded. Double-click on `quickinstall` and it will compile the app for you, then ask if you'd like to install it system-wide. If the build fails for any reason or if you'd like to compile manually, see `BUILD.md`.

Usage
-----

The user interface is still a work in progress. See `DAEMON.md` for info about the daemon program.

**Major features:**
- Control multiple keyboards independently (note: not tested)
- United States and European keyboard layouts
- Customizable key bindings
- Per-key lighting and animation
- Reactive lighting
- Multiple profiles/modes with hardware save function

**Roadmap** (roughly in order)
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

Troubleshooting
---------------

#### Linux

If you have problems connecting the keyboard to your system (keyboard doesn't respond, ckb-daemon doesn't recognize it or can't connect the keyboard), try adding the following to your kernel's `cmdline`:

* K65 RGB: `usbhid.quirks=0x1B1C:0x1B17:0x400`
* K70: `usbhid.quirks=0x1B1C:0x1B09:0x400`
* K70 RGB: `usbhid.quirks=0x1B1C:0x1B13:0x400`
* K95: `usbhid.quirks=0x1B1C:0x1B08:0x400`
* K95 RGB: `usbhid.quirks=0x1B1C:0x1B11:0x408`

For instructions on adding `cmdline` parameters in Ubuntu, see https://wiki.ubuntu.com/Kernel/KernelBootParameters

If the keyboard still doesn't work, try replacing `0x400`/`0x408` with `0x4`. Note that this will cause the kernel driver to ignore the keyboard completely, so you'll need to make sure ckb-daemon is running at boot or else you'll have no keyboard input.

#### OSX

Compile problems can usually be resolved by rebooting your computer and/or reinstalling Qt. Make sure that Xcode works on its own. If a compile fails, delete the `ckb-master` directory as well as any automatically generated `build-ckb` folders and try again from a new download.

If you've rebound your modifier keys in System Preferences, the changes will not be recognized anymore. You can rebind them again within the application.

#### General

**Please note: the keyboards do have some hardware-related problems which Corsair is aware of and working to fix. These are out of my control. Before reporting an issue, connect your keyboard to a Windows computer first and see if your problem happens in Windows. If it does, contact Corsair.** Additionally, please check the Corsair user forums to see if your issue has been reported by other users. If so, try their solutions first.

Connection issues can sometimes be solved by rebooting. Make sure that your system is up to date. If possible, update your computer's BIOS to the latest version as well.

Common problems:
- **Problem:** Keyboard doesn't work in BIOS, doesn't work at boot
- **Solution:** Some BIOSes have trouble communicating with the keyboard. They may prevent the keyboard from working correctly in the operating system as well. First, try booting the OS *without* the keyboard attached, and plug the keyboard in after logging in. If the keyboard works after the computer is running but does not work at boot, you may need to use the keyboard's BIOS mode option.
- BIOS mode can be activated using the poll rate switch at the back of the keyboard. Slide it all the way to the position marked "BIOS". You should see the scroll lock light blinking to indicate that it is on. (Note: Unfortunately, this has its own problems - see Known Issues. You may need to activate BIOS mode when booting the computer and deactivate it after logging in).
- **Problem:** Keyboard isn't detected when plugged in, even if driver is already running
- **Solution:** Try moving to a different USB port. Be sure to follow [Corsair's USB connection requirements](http://forum.corsair.com/v3/showthread.php?t=132322). Note that the keyboard does not work with some USB3 controllers - if you have problems with USB3 ports, try USB2 instead. If you have any USB hubs on hand, try those as well. You may also have success sliding the poll switch back and forth a few times.

Known issues
------------

- Using the keyboard in BIOS mode prevents the media keys (including mute and volume wheel) from working. This is a hardware limitation.
- In BIOS mode, keys sometimes get "stuck" or stop responding. It appears to be a firmware bug. Using the keyboard in BIOS mode while running ckb is not recommended.
- Animations sometimes cause the keyboard to freeze or cause keys to be dropped. These also seem to be firmware bugs; the best way to mitigate them is to decrease the frame rate in ckb or increase the polling interval on the keyboard.
- The tray icon doesn't always appear even when enabled. Apparently this is a known Qt bug. If you can't see the icon, reopen ckb to bring the window back.
- When starting the driver manually on OSX, the Terminal window sometimes gets spammed with enter keys. You can stop it by unplugging and replugging the keyboard or by moving the poll rate switch.
- When stopping the driver manually, the keyboard sometimes stops working completely. This seems to be a hardware/OS communication issue.
