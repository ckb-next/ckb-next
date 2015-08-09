ckb: Corsair Input Driver for Linux and OSX
===========================================

**ckb** is an open-source driver for Corsair keyboards and mice. It aims to bring the features of their proprietary software to the Linux and Mac operating systems. This project is currently a work in progress but already supports much of the same functionality, including full RGB animations. More features are coming soon. Testing and bug reports are appreciated!

**Disclaimer:** ckb is not an official Corsair product. It is licensed under the GNU General Public License (version 2) in the hope that it will be useful, but with NO WARRANTY of any kind.

If you use and enjoy this project, I'd appreciate if you could spare a few dollars for a donation. This is completely voluntary - the project will remain free and open source regardless. `:)`

I accept donations through PayPal: [Click here](https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=DCLHFH9S3KZ8W&lc=US&item_name=ckb&item_number=ckb%20GitHub%20Page&no_note=1&no_shipping=1&currency_code=USD&bn=PP%2dDonationsBF%3abtn_donateCC_LG%2egif%3aNonHosted)

Or through Bitcoin: 1LDHCfyDqAziUPtp3a4BdUaKmwphG1yroQ [(click here for QR code)](https://i.imgur.com/h3gyLiv.png)

Contents
--------

* [Device Support](#device-support)
* [Linux Installation](#linux-installation)
* [OSX Installaction](#osx-installation)
* [Usage](#usage)
* [Troubleshooting](#troubleshooting)
* [Known Issues](#known-issues)
* [Contributing](#contributing)

See also:

* [Manual for the driver daemon](https://github.com/ccMSC/ckb/blob/master/DAEMON.md)
* [ckb testing repository](https://github.com/ccMSC/ckb/tree/testing) (updated more frequently, but may be unstable)

Device Support
--------------

ckb currently supports the following Corsair devices:

* K65 RGB keyboard
* K70 keyboard
* K70 RGB keyboard
* K95* keyboard
* K95 RGB keyboard
* M65 RGB mouse

\* = hardware playback not yet supported. Settings will be saved to software only.

Linux Installation
------------------

Requires Qt5, libudev, zlib, gcc, g++, and glibc (Ubuntu: `qt5-default`, `libudev-dev`, `build-essential`, `zlib1g-dev`. In other distros, Qt5 may be known as `qt5-base` or `libqt5*-devel`). Check with your package manager to make sure you have the correct libraries/headers installed.

You can download ckb using the "Download zip" option on the right. Extract it and open the ckb-master directory. The easiest way to install ckb is to double-click the `quickinstall` script and run it in a Terminal. It will attempt to build the application, and if all goes well, will ask if you'd like to install/run ckb on your system (press enter to proceed; default answer is "yes"). If the build doesn't succeed, or if you'd like to build ckb manually, see [`BUILD.md`](https://github.com/ccMSC/ckb/blob/master/BUILD.md) for instructions.

#### Reinstalling:

To install a new version of ckb, or to reinstall the same version, first delete the ckb-master directory and the zip file from your previous download. Then download the source code again and re-run `quickinstall`. The script will automatically replace the previous installation. You may need to reboot afterward.

#### Uninstalling:

First, stop the ckb-daemon service and remove the service file.
* If you have systemd (Ubuntu versions starting with 15.04):
```
sudo systemctl stop ckb-daemon
sudo rm -f /usr/lib/systemd/system/ckb-daemon.service
```
* If you have Upstart (Ubuntu versions earlier than 15.04):
```
sudo service ckb-daemon stop
sudo rm -f /etc/init/ckb-daemon.conf
```
* If you're not sure, re-run the quickinstall script and it will tell you when it installs itself as a service. The script will say `System service: Upstart detected` or `System service: systemd detected`

Afterward, remove the applications and related files:
```
sudo rm -f /usr/bin/ckb /usr/bin/ckb-daemon /usr/share/applications/ckb.desktop /usr/share/icons/hicolor/512x512/apps/ckb.png
sudo rm -rf /usr/bin/ckb-animations
```

OSX Installation
----------------

#### Binary download:

The latest OSX binary can be downloaded here: https://github.com/ccMSC/ckb/releases/latest

Click on `ckb.pkg` under the Downloads section. This is an automated installer which will set up the driver for you. After it's finished, open ckb.app (it will be installed to your Applications directory) to get started.

#### Building from source:

Install the latest version of Xcode from the App Store. Open Xcode, accept the license agreement, and wait for it to install any additional components (if necessary). When you see the "Welcome to Xcode" screen, the setup is finished and you can close the app. Then install Qt5 from here: http://www.qt.io/download-open-source/

The easiest way to build the driver is with the `quickinstall` script, which is present in the ckb-master folder. Double-click on `quickinstall` and it will compile the app for you, then ask if you'd like to install it system-wide. If the build fails for any reason or if you'd like to compile manually, see [`BUILD.md`](https://github.com/ccMSC/ckb/blob/master/BUILD.md).

#### Reinstalling (binary):

Download the latest `ckb.pkg`, run the installer, and reboot. The newly-installed driver will replace the old one.

#### Reinstalling (source):

Remove the existing ckb-master directory and zip file. Re-download the source code and run the `quickinstall` script again. The script will automatically replace the previous installation. You may need to reboot afterward.

#### Uninstalling:

Drag `ckb.app` into the trash. If the system service file isn't cleaned up automatically, you can find it and remove it here: `/Library/LaunchDaemons/com.ckb.daemon.plist`.

Usage
-----

The user interface is still a work in progress.

**Major features:**
- Control multiple devices independently
- United States and European keyboard layouts
- Customizable key bindings
- Per-key lighting and animation
- Reactive lighting
- Multiple profiles/modes with hardware save function
- Adjustable mouse DPI with ability to change DPI on button press

**Roadmap** (roughly in order)
- **v0.3 release:**
- Additional lighting options
- More functions for the Win Lock key
- Key combos
- Key macros, other advanced keypress features like running a custom command
- (Daemon) Macros with timing info, delays, repeats
- (Daemon) Notification macros
- **v0.4 release:**
- Ability to store profiles separately from devices, import/export them
- Ability to tie profiles to which application has focus
- Timers
- **v0.5 release:**
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

```
usbhid.quirks=0x1B1C:0x1B08:0x408,0x1B1C:0x1B09:0x400,0x1B1C:0x1B11:0x408,0x1B1C:0x1B12:0x400,0x1B1C:0x1B13:0x400,0x1B1C:0x1B17:0x400
```

This should apply the correct fix for all supported devices. For instructions on adding `cmdline` parameters in Ubuntu, see https://wiki.ubuntu.com/Kernel/KernelBootParameters

If the keyboard still doesn't work, try this instead:

```
usbhid.quirks=0x1B1C:0x1B08:0x4,0x1B1C:0x1B09:0x4,0x1B1C:0x1B11:0x4,0x1B1C:0x1B12:0x4,0x1B1C:0x1B13:0x4,0x1B1C:0x1B17:0x4
```
Note that this will cause the kernel driver to ignore the keyboard completely, so you need to ensure ckb-daemon is running at boot or else you'll have no keyboard input.

If you're using **Unity** and the tray icon doesn't appear correctly, install the following package: `libappindicator-dev`. Then reinstall ckb.

#### OSX

- **“ckb.pkg” can’t be opened because it is from an unidentified developer.**
- Open `System Preferences > Security & Privacy > General` and click `Open Anyway`.
- **Modifier keys (Shift, Ctrl, etc) are not rebound correctly.**
- ckb does not recognize modifier keys rebound from System Preferences. You can rebind them again within the application.
- **Compile problems** can usually be resolved by rebooting your computer and/or reinstalling Qt. Make sure that Xcode works on its own. If a compile fails, delete the `ckb-master` directory as well as any automatically generated `build-ckb` folders and try again from a new download.

#### General

**Please ensure your keyboard firmware is up to date. If you've just bought the keyboard, connect it to a Windows computer first and update the firmware from Corsair's official utility.**

**Before reporting an issue, connect your keyboard to a Windows computer and see if the problem still occurs. If it does, contact Corsair.** Additionally, please check the Corsair user forums to see if your issue has been reported by other users. If so, try their solutions first.

Common issues:
- **Problem:** ckb says "No devices connected" or "Driver inactive"
- **Solution:** Try rebooting the computer and/or reinstalling ckb. Try removing the keyboard and plugging it back in. If the error doesn't go away, try the following:
- **Problem:** Keyboard doesn't work in BIOS, doesn't work at boot
- **Solution:** Some BIOSes have trouble communicating with the keyboard. They may prevent the keyboard from working correctly in the operating system as well. First, try booting the OS *without* the keyboard attached, and plug the keyboard in after logging in. If the keyboard works after the computer is running but does not work at boot, you may need to use the keyboard's BIOS mode option.
- BIOS mode can be activated using the poll rate switch at the back of the keyboard. Slide it all the way to the position marked "BIOS". You should see the scroll lock light blinking to indicate that it is on. (Note: Unfortunately, this has its own problems - see Known Issues. You may need to activate BIOS mode when booting the computer and deactivate it after logging in).
- **Problem:** Keyboard isn't detected when plugged in, even if driver is already running
- **Solution:** Try moving to a different USB port. Be sure to follow [Corsair's USB connection requirements](http://forum.corsair.com/v3/showthread.php?t=132322). Note that the keyboard does not work with some USB3 controllers - if you have problems with USB3 ports, try USB2 instead. If you have any USB hubs on hand, try those as well. You may also have success sliding the poll switch back and forth a few times.

#### Reporting issues

If you have a problem that you can't solve (and it isn't mentioned in the Known Issues section below), you can report it on [the GitHub issue tracker](https://github.com/ccMSC/ckb/issues) or [the Corsair User Forum thread](http://forum.corsair.com/v3/showthread.php?t=133929). I usually try to respond within 48 hours. Before opening a new issue, please check to see if someone else has reported your problem already - if so, feel free to leave a comment there.

Known issues
------------

- Using the keyboard in BIOS mode prevents the media keys (including mute and volume wheel), as well as the K95's G-keys from working. This is a hardware limitation.
- The tray icon doesn't appear in some desktop environments. This is a known Qt bug. If you can't see the icon, reopen ckb to bring the window back.
- When starting the driver manually, the Terminal window sometimes gets spammed with enter keys. You can stop it by unplugging and replugging the keyboard or by moving the poll rate switch.
- When stopping the driver manually, the keyboard sometimes stops working completely. You can reconnect it by moving the poll rate switch.

Contributing
------------

You can contribute to the project by [opening a pull request](https://github.com/ccMSC/ckb/pulls). It's best if you base your changes off of the `testing` branch as opposed to the `master`, because the pull request will be merged there first. If you'd like to contribute but don't know what you can do, take a look at [the issue tracker](https://github.com/ccMSC/ckb/issues) and see if any features/problems are still unresolved. Feel free to ask if you'd like some ideas.
