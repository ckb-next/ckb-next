# ckb-next: RGB Driver for Linux and macOS

<a target="_blank" href="http://webchat.freenode.net?channels=%23ckb-next&uio=d4"><img src="https://cloud.githubusercontent.com/assets/493242/14886493/5c660ea2-0d51-11e6-8249-502e6c71e9f2.png" height = "20" /></a>

**ckb-next** is an open-source driver for Corsair keyboards and mice. It aims to bring some features of [Corsair Utility Engine](http://www.corsair.com/en-us/landing/cue) to the Linux and Mac operating systems.

![Screenshot](https://i.imgur.com/zMK9jOP.png)

***Disclaimer:** ckb-next is not an official Corsair product. It is licensed under the GNU General Public License (version 2) in the hope that it will be useful, but with NO WARRANTY of any kind.*

Curious how *ckb-next* appeared? See [Project's History](docs/PROJECTS_HISTORY.md).

### Current Status

Right now ckb-next is under active development. *We will cut a release and ship a macOS binary as soon as some important changes are done*. You can always build the software from source with just one command. See [Linux Installation](#linux-installation) and [macOS Installation](#macos-installation). Thank you for the interest in this software and your patience.

### Device Support

See the list of [Supported Hardware](docs/SUPPORTED_HARDWARE.md).

### Installation

***Warning**: this is the `cmake` branch and thus instructions here describe what's best from CMake's perspective. If you're looking for qmake version, you need the `master` branch. If you don't know which one to choose, the original author of this document highly recommends CMake.*

#### Linux Installation

Follow the instructions [here](docs/LINUX_INSTALLATION.md).

#### macOS Installation

Follow the instructions [here](docs/MACOS_INSTALLATION.md).

#### Binary download:

The latest OS X/macOS binary can be downloaded here: https://github.com/mattanger/ckb-next/releases/latest

Click on `ckb-next.pkg` under the Downloads section. This is an automated installer which will set up the driver for you. After it's finished, open ckb.app (it will be installed to your Applications directory) to get started.

#### Building from source:

Install the latest version of Xcode from the App Store. Open Xcode, accept the license agreement, and wait for it to install any additional components (if necessary). When you see the "Welcome to Xcode" screen, the setup is finished and you can close the app. Then install Xcode Command Line Tools package issuing `xcode-select --install` in a Terminal app. Afterwards install Qt5 from here: http://www.qt.io/download-open-source/

The easiest way to build the driver is with the `quickinstall` script, which is present in the ckb-master folder. Double-click on `quickinstall` and it will compile the app for you, then ask if you'd like to install it system-wide. If the build fails for any reason, or if you'd like to compile manually, see [`BUILD.md`](https://github.com/ccMSC/ckb/blob/master/BUILD.md).

#### Upgrading (binary):

Download the latest `ckb.pkg`, run the installer, and reboot. The newly-installed driver will replace the old one.

#### Upgrading (source):

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

Closing ckb will actually minimize it to the system tray. Use the Quit option from the tray icon or the settings screen to exit the application.

**Roadmap** (roughly in order)
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

Troubleshooting
---------------

#### Linux

If you have problems connecting the device to your system (device doesn't respond, ckb-daemon doesn't recognize or can't connect it) and/or you experience long boot times when using the keyboard, try adding the following to your kernel's `cmdline`:

* K65 RGB: `usbhid.quirks=0x1B1C:0x1B17:0x20000408`
* K70: `usbhid.quirks=0x1B1C:0x1B09:0x0x20000408`
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
`GLib-GObject-CRITICAL **: g_type_add_interface_static: assertion 'G_TYPE_IS_INSTANTIATABLE (instance_type)' failed` 
and you are using:

- Qt 5.8 and newer, remove your Qt configuration files and restart the ckb GUI. Also watch out for different style overridings in dotfiles under `~/` generated by Qt automatically, remove them as well.

- Qt 5.7 and lower, install `qt5ct` package on Arch Linux (find a similar one for your distribution). That's all. This is a known Qt bug. It happened because Qt did not ship required GTK files.

If you're using **Unity** and the tray icon doesn't appear correctly, run `sudo apt-get install libappindicator-dev`. Then reinstall ckb.

#### OS X/macOS

- **“ckb.pkg” can’t be opened because it is from an unidentified developer**
    Open `System Preferences > Security & Privacy > General` and click `Open Anyway`.
- **Modifier keys (Shift, Ctrl, etc.) are not rebound correctly**
    ckb does not recognize modifier keys rebound from System Preferences. You can rebind them again within the application.
- **`~` key prints `§±`**
    Check your keyboard layout on ckb's Settings screen. Choose the layout that matches your physical keyboard.
- **Compile problems**
    Can usually be resolved by rebooting your computer and/or reinstalling Qt. Make sure that Xcode works on its own. If a compile fails, delete the `ckb-master` directory as well as any automatically generated `build-ckb` folders and try again from a new download.
- **Scroll wheel does not scroll**
    As of #c3474d2 it's now possible to **disable scroll acceleration** from the GUI. You can access it under "OSX tweaks" in the "More settings" screen. Once disabled, the scroll wheel should behave consistently.



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

If you have a problem that you can't solve (and it isn't mentioned in the Known Issues section below), you can report it on [the GitHub issue tracker](https://github.com/mattanger/ckb-next/issues). Before opening a new issue, please check to see if someone else has reported your problem already - if so, feel free to leave a comment there.

Known issues
------------

- Using the keyboard in BIOS mode prevents the media keys (including mute and volume wheel), as well as the K95's G-keys from working. This is a hardware limitation.
- The tray icon doesn't appear in some desktop environments. This is a known Qt bug. If you can't see the icon, reopen ckb to bring the window back.
- When starting the driver manually, the Terminal window sometimes gets spammed with enter keys. You can stop it by unplugging and replugging the keyboard or by moving the poll rate switch.
- When stopping the driver manually, the keyboard sometimes stops working completely. You can reconnect it by moving the poll rate switch.

Contributing
------------

You can contribute to the project by [opening a pull request](https://github.com/mattanger/ckb-next/pulls). It's best if you base your changes off of the `testing` branch as opposed to the `master`, because the pull request will be merged there first. If you'd like to contribute but don't know what you can do, take a look at [the issue tracker](https://github.com/mattanger/ckb-next/issues) and see if any features/problems are still unresolved. Feel free to ask if you'd like some ideas.
~^^~