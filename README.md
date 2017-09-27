# ckb-next: RGB Driver for Linux and macOS

**ckb-next** is an open-source driver for Corsair keyboards and mice. It aims to bring the features of their proprietary CUE software to the Linux and Mac operating systems. This project is currently a work in progress, but it already supports much of the same functionality, including full RGB animations. More features are coming soon. Testing and bug reports are appreciated!

![Screenshot](https://i.imgur.com/zMK9jOP.png)

**Disclaimer:** ckb-next is not an official Corsair product. It is licensed under the GNU General Public License (version 2) in the hope that it will be useful, but with NO WARRANTY of any kind.

<!-- TOC depthFrom:2 -->

- [Device Support](#device-support)
    - [Keyboards](#keyboards)
    - [Mice](#mice)
- [Linux Installation](#linux-installation)
    - [Pre-made packages](#pre-made-packages)
    - [Preparation](#preparation)
    - [Installing](#installing)
    - [Upgrading](#upgrading)
    - [Uninstalling](#uninstalling)
- [OS X/macOS Installation](#os-xmacos-installation)
    - [Binary download](#binary-download)
    - [Building from source](#building-from-source)
    - [Upgrading (binary)](#upgrading-binary)
    - [Upgrading (source)](#upgrading-source)
    - [Uninstalling](#uninstalling-1)
- [Usage](#usage)
    - [Major features](#major-features)
    - [Roadmap](#roadmap)
- [Troubleshooting](#troubleshooting)
    - [Linux](#linux)
    - [OS X/macOS](#os-xmacos)
    - [General](#general)
    - [Reporting issues](#reporting-issues)
- [Known issues](#known-issues)
- [Contributing](#contributing)
- [Contact us](#contact-us)
- [What happened to the original ckb](#what-happened-to-the-original-ckb)

<!-- /TOC -->

See also:

* [Manual for the driver daemon](https://github.com/mattanger/ckb-next/blob/master/DAEMON.md)

## Device Support

### Keyboards

* K65:
    * RGB
    * non-RGB
    * LUX RGB
    * RGB RAPIDFIRE
* K70:
    * RGB
    * non-RGB
    * LUX RGB
    * LUX non-RGB
    * RGB RAPIDFIRE
    * non-RGB RAPIDFIRE
* K95:
    * RGB
    * non-RGB\*
* Strafe:
    * RGB
    * non-RGB

\* = hardware playback not supported. Settings will be saved to software only.

### Mice

* M65:
    * non-RGB
    * PRO RGB
* Sabre:
    * Optical RGB
    * Laser RGB
* Scimitar:
    * RGB
    * PRO RGB

## Linux Installation

### Pre-made packages

* Fedora 24/25, CentOS/RHEL 7 (maintained by [@hevanaa](https://github.com/hevanaa)):
    * [`johanh/ckb`](https://copr.fedorainfracloud.org/coprs/johanh/ckb/) - based on `master` branch
* Arch Linux (maintained by [@makz27](https://github.com/makz27), [@light2yellow](https://github.com/light2yellow)):
    * [`aur/ckb-next`](https://aur.archlinux.org/packages/ckb-next) - based on GitHub releases
    * [`aur/ckb-next-git`](https://aur.archlinux.org/packages/ckb-next-git) - based on `master` branch
    * [`aur/ckb-next-latest-git`](https://aur.archlinux.org/packages/ckb-next-latest-git) - based on `newdev` branch

If you are a package maintainer or want to discuss something with package maintainers let us know in [#5](https://github.com/mattanger/ckb-next/issues/5), so we can have an accountable and centralized communication about this. *If you would like to maintain a package for your favorite distro/OS, please let us know as well.*

### Preparation

ckb-next requires Qt5 (Qt 5.9 is recommended), libudev, zlib, gcc, g++, and glibc.

* Ubuntu: `sudo apt-get install build-essential libudev-dev qt5-default zlib1g-dev libappindicator-dev`
* Fedora: `sudo dnf install zlib-devel qt5-qtbase-devel libgudev-devel libappindicator-devel systemd-devel gcc-c++`
* Arch: `sudo pacman -S base-devel qt5-base zlib`
* Other distros: Look for `qt5` or `libqt5*-devel`

Note: If you build your own kernels, ckb-next requires the `CONFIG_INPUT_UINPUT` flag to be enabled. It is located in `Device Drivers -> Input Device Support -> Miscellaneous devices -> User level driver support`. If you don't know what this means, you can ignore this.

### Installing

You can download ckb-next using the "Download zip" option on the right or clone it using `git clone`. Extract it and open the ckb-master directory in a terminal. Run `./quickinstall`. It will attempt to build ckb and then ask if you'd like to install/run the application. If the build doesn't succeed, or if you'd like to hand-tune the compilation of ckb, see [`BUILD.md`](https://github.com/mattanger/ckb-next/blob/master/BUILD.md) for instructions.

### Upgrading

To install a new version of ckb, or to reinstall the same version, first delete the ckb-master directory and the zip file from your previous download. Then download the source code again and re-run `./quickinstall`. The script will automatically replace the previous installation. You may need to reboot afterward.

### Uninstalling

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
* If you have OpenRC:
```
sudo rc-service ckb-daemon stop
sudo rc-update del ckb-daemon default
sudo rm -f /etc/init.d/ckb-daemon
```
* If you're not sure, re-run the `quickinstall` script and proceed to the service installation. The script will say `System service: Upstart detected` or `System service: systemd detected`. Please be aware that OpenRC is currently not detected automatically.

Afterward, remove the applications and related files:
```
sudo rm -f /usr/bin/ckb /usr/bin/ckb-daemon /usr/share/applications/ckb.desktop /usr/share/icons/hicolor/512x512/apps/ckb.png
sudo rm -rf /usr/lib/ckb-animations
```

Before https://github.com/mattanger/ckb-next/commit/f347e60df211c60452f95084b6c46dc4ec5f42ee animations were located elsewhere, try removing them as well:
```
sudo rm -rf /usr/bin/ckb-animations
```

## OS X/macOS Installation

### Binary download

macOS `pkg` can be downloaded from [GitHub Releases](https://github.com/mattanger/ckb-next/releases). It is always built with the last available stable Qt version and tagrets 10.10 SDK. If you run 10.9.x, you'll need to build the project from source and comment out `src/ckb-heat` (and the backslash above it) inside `ckb.pro`.

### Building from source

Install the latest version of Xcode from the App Store. While it's downloading, open the Terminal and execute `xcode-select --install` to install Command Line Tools. Then open Xcode, accept the license agreement and wait for it to install any additional components (if necessary). When you see the "Welcome to Xcode" screen, from the top bar choose `Xcode -> Preferences -> Locations -> Command Line Tools` and select an SDK version. Afterwards install [Homebrew](https://brew.sh/) and execute `brew install qt5` in the Terminal.

> __Note__: If you decide to use the official Qt5 package from Qt website instead, you will have to edit the installation script and provide installation paths manually due to a qmake bug.

The easiest way to build the driver is with the `quickinstall` script, which is present in the ckb-master folder. Double-click on `quickinstall` and it will compile the app for you, then ask if you'd like to install it system-wide. If the build fails for any reason, or if you'd like to compile and install manually, see [`BUILD.md`](https://github.com/ccMSC/ckb/blob/master/BUILD.md).

### Upgrading (binary)

Download the latest `ckb.pkg`, run the installer, and reboot. The newly-installed driver will replace the old one.

### Upgrading (source)

Remove the existing ckb-master directory and zip file. Re-download the source code and run the `quickinstall` script again. The script will automatically replace the previous installation. You may need to reboot afterward.

### Uninstalling

Drag `ckb.app` into the trash. Then stop and remove the agent:

```sh
sudo unload /Library/LaunchDaemons/com.ckb.daemon.plist
sudo rm /Library/LaunchDaemons/com.ckb.daemon.plist
```

## Usage

The user interface is still a work in progress.

### Major features

- Control multiple devices independently
- United States and European keyboard layouts
- Customizable key bindings
- Per-key lighting and animation
- Reactive lighting
- Multiple profiles/modes with hardware save function
- Adjustable mouse DPI with ability to change DPI on button press

Closing ckb will actually minimize it to the system tray. Use the Quit option from the tray icon or the settings screen to exit the application.

### Roadmap

- **v0.3 release:**
- Ability to store profiles separately from devices, import/export them
- More functions for the Win Lock key
- Key macros
- **v0.4 release:**
- Ability to import CUE profiles
- Ability to tie profiles to which application has focus
- **v0.5 release:**
- Key combos
- Timers?
- **v1.0 release:**
- OSD? (Not sure if this can actually be done)
- Extra settings?
- ????

## Troubleshooting

### Linux

If you have problems connecting the device to your system (device doesn't respond, ckb-daemon doesn't recognize or can't connect it) and/or you experience long boot times when using the keyboard, try adding the following to your kernel's `cmdline`:

* K65 RGB: `usbhid.quirks=0x1B1C:0x1B17:0x20000408`
* K70: `usbhid.quirks=0x1B1C:0x1B09:0x20000408`
* K70 LUX: `usbhid.quirks=0x1B1C:0x1B36:0x20000408`
* K70 RGB: `usbhid.quirks=0x1B1C:0x1B13:0x20000408`
* K95: `usbhid.quirks=0x1B1C:0x1B08:0x20000408`
* K95 RGB: `usbhid.quirks=0x1B1C:0x1B11:0x20000408`
* Strafe: `usbhid.quirks=0x1B1C:0x1B15:0x20000408`
* Strafe RGB: `usbhid.quirks=0x1B1C:0x1B20:0x20000408`
* M65 RGB: `usbhid.quirks=0x1B1C:0x1B12:0x20000408`
* Sabre RGB Optical: `usbhid.quirks=0x1B1C:0x1B14:0x20000408`
* Sabre RGB Laser: `usbhid.quirks=0x1B1C:0x1B19:0x20000408`
* Scimitar RGB: `usbhid.quirks=0x1B1C:0x1B1E:0x20000408`

For instructions on adding `cmdline` parameters in Ubuntu, see https://wiki.ubuntu.com/Kernel/KernelBootParameters

If you have multiple devices, combine them with commas, starting after the `=`. For instance, for K70 RGB + M65 RGB: `usbhid.quirks=0x1B1C:0x1B13:0x20000408,0x1B1C:0x1B12:0x20000408`

If it still doesn't work, try replacing `0x20000408` with `0x4`. Note that this will cause the kernel driver to ignore the device(s) completely, so you need to ensure ckb-daemon is running at boot or else you'll have no input. This will not work if you are using full-disk encryption.

If you see **GLib** critical errors like
```
GLib-GObject-CRITICAL **: g_type_add_interface_static: assertion 'G_TYPE_IS_INSTANTIATABLE (instance_type)' failed
```
read [this Arch Linux thread](https://bbs.archlinux.org/viewtopic.php?id=214147) and try different combinations from it. If it doesn't help, you might want get support from your distribution community and tell them you cannot solve the problem in this thread.

If you're using **Unity** and the tray icon doesn't appear correctly, run `sudo apt-get install libappindicator-dev`. Then reinstall ckb.

#### Fedora 26 Color Changer Freeze Fix

If you're running Fedora 26, a working solution for the color changer freezing issue is to install qt5ct `dnf install qt5ct` then modify your /etc/environment file to contain the line `QT_QPA_PLATFORMTHEME=qt5ct`

### OS X/macOS

- **“ckb.pkg” can’t be opened because it is from an unidentified developer**
    Right-click (control-click) on ckb.pkg and select Open. This new dialog box will give you the option to open anyway, without changing your system preferences.
- **Modifier keys (Shift, Ctrl, etc.) are not rebound correctly**
    ckb does not recognize modifier keys rebound from System Preferences. You can rebind them again within the application.
- **`~` key prints `§±`**
    Check your keyboard layout on ckb's Settings screen. Choose the layout that matches your physical keyboard.
- **Compile problems**
    Can usually be resolved by rebooting your computer and/or reinstalling Qt. Make sure that Xcode works on its own. If a compile fails, delete the `ckb-master` directory as well as any automatically generated `build-ckb` folders and try again from a new download.
- **Scroll wheel does not scroll**
    As of #c3474d2 it's now possible to **disable scroll acceleration** from the GUI. You can access it under "OSX tweaks" in the "More settings" screen. Once disabled, the scroll wheel should behave consistently.

### General

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

### Reporting issues

If you have a problem that you can't solve (and it isn't mentioned in the Known Issues section below), you can report it on [the GitHub issue tracker](https://github.com/mattanger/ckb-next/issues). Before opening a new issue, please check to see if someone else has reported your problem already - if so, feel free to leave a comment there.

## Known issues

- Using the keyboard in BIOS mode prevents the media keys (including mute and volume wheel), as well as the K95's G-keys from working. This is a hardware limitation.
- The tray icon doesn't appear in some desktop environments. This is a known Qt bug. If you can't see the icon, reopen ckb to bring the window back.
- When starting the driver manually, the Terminal window sometimes gets spammed with enter keys. You can stop it by unplugging and replugging the keyboard or by moving the poll rate switch.
- When stopping the driver manually, the keyboard sometimes stops working completely. You can reconnect it by moving the poll rate switch.
- On newer versions of macOS (i.e. 10.12 and up) CMD/Shift+select does not work, yet. Stopping the daemon and GUI for `ckb` will fix this issue temporarily.

## Contributing

You can contribute to the project by [opening a pull request](https://github.com/mattanger/ckb-next/pulls). It's best if you base your changes off of the `testing` branch as opposed to the `master`, because the pull request will be merged there first. If you'd like to contribute but don't know what you can do, take a look at [the issue tracker](https://github.com/mattanger/ckb-next/issues) and see if any features/problems are still unresolved. Feel free to ask if you'd like some ideas.

## Contact us

There are multiple ways you can get in touch with us:

* [join](https://groups.google.com/forum/#!forum/ckb-next/join) `ckb-next` mailing list
* [open](https://github.com/mattanger/ckb-next/issues) a GitHub Issue
* hop on `#ckb-next` to chat <a target="_blank" href="http://webchat.freenode.net?channels=%23ckb-next&uio=d4"><img src="https://cloud.githubusercontent.com/assets/493242/14886493/5c660ea2-0d51-11e6-8249-502e6c71e9f2.png" height = "20" /></a>

## What happened to the original ckb

Due to time restrictions, the original author of **ckb** [ccMSC](https://github.com/ccMSC) hasn't been able to further develop the software. So the community around it decided to take the project over and continue its development. That's how **ckb-next** was created. Currently it's not rock solid and not very easy to set up on newer systems but we are actively working on this. Nevertheless the project already incorporates a notable amount of fixes and patches in comparison to the original ckb.
