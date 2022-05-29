# Change Log

## [v0.5.0](https://github.com/ckb-next/ckb-next/tree/v0.5.0) (2022-05-27)
[Full Changelog](https://github.com/ckb-next/ckb-next/compare/v0.4.4...v0.5.0)

Support for new devices:
- K95 Platinum XT
- Katar Pro
- Katar Pro XT
- Glaive Pro
- M55
- K60 Pro RGB
- K60 Pro RGB Low Profile
- K60 Pro RGB SE

Important bugfixes:
- Scroll wheels are now treated as axes (Responsiveness should be improved for specific mice)
- The lights on the K95 RGB Platinum top bar are now updated correctly
- An infinite loop is prevented if certain USB information can not be read
- GUI no longer crashes on exit under certain conditions
- Mouse scrolling works again when combined with specific libinput versions
- The daemon no longer hangs when quitting due to LED keyboard indicators
- The lighting programming key can now be rebound on K95 Legacy
- Animations won't break due to daylight savings / system time changes
- GUI doesn't crash when switching to a hardware mode on a fresh installation
- Daemon no longer causes a kernel Oops on resume under certain conditions (Devices now resume correctly from sleep)
- Window detection is more reliable and works correctly on system boot
- Settings tab now stretches correctly
- Profile switch button can now be bound correctly on mice
- ISO Enter key is now aligned correctly
- Bindings are now consistent between demo and new modes
- Firmware update dialog is no longer cut off and can be resized
- RGB data won't be sent to the daemon when brightness is set to 0%

New features:
- German translation
- 66 service (not installed automatically)
- Device previews are now resizable

## [v0.4.4](https://github.com/ckb-next/ckb-next/tree/v0.4.4) (2021-03-18)
[Full Changelog](https://github.com/ckb-next/ckb-next/compare/v0.4.3...v0.4.4)

Important bugfixes:
- Fixed a crash when creating a new profile
- Fixed lights turning off unexpectedly or not being restored correctly
- Macros can now be repeated quickly by repeatedly pressing the keys
- Fixed various deadlocks when resuming from suspend or during device initialisation
- Fixed devices not functioning during system boot requiring a daemon restart

## [v0.4.3](https://github.com/ckb-next/ckb-next/tree/v0.4.3) (2020-12-18)
[Full Changelog](https://github.com/ckb-next/ckb-next/compare/v0.4.2...v0.4.3)

PLEASE NOTE: This is the last version of ckb-next to officially support macOS.
It has been reported to work with Big Sur after disabling SIP.
https://github.com/ckb-next/ckb-next/issues/660
ARM Macs are NOT supported.

Support for new devices:
- Scimitar RGB Elite
- Nightsword RGB

Important bugfixes:
- Fixed delay when shutting down with K95 RGB
- Fixed freezing when updating indicator LEDs
- Indicator LEDs are now more responsive when an animation is playing
- GUI now starts hidden in tray on KDE
- The pipe animation now works consistently between mode changes
- The pipe animation now handles input commands sent in rapid succession before the device is updated
- Input works on wayland again after recent updates
- libgdx based games no longer crash on mouse input
- Mouse settings are now correctly restored when resuming from suspend
- Fixed a bug where some devices would not be initialised on daemon start and would require a replug
- Fixed layouts for K68, K65, K63, M95

New features:
- Under X11 only, lights can now automatically turn off after a user-set time
- Macros now loop when the key is held down
- Macro UI has been redesigned and supports recording from non ckb-next managed keyboards
- Modes can now be changed automatically based on the current focused application (X11/XWayland Only)
- Translations are now supported

Notes for packagers:
- The systemd unit directory can be overriden with -DSYSTEMD_UNIT_INSTALL_DIR
- libappindicator is no longer used
- Minimum Qt version has been bumped up to 5.5.1 (xenial)
- cmake now checks for quazip 1.0
- The following new dependencies have been introduced: Qt5 X11Extras, xcb, xcb-screensaver, xcb-ewmh, Qt5 translation tools, dbusmenu-qt5

## [v0.4.2](https://github.com/ckb-next/ckb-next/tree/v0.4.2) (2019-10-08)
[Full Changelog](https://github.com/ckb-next/ckb-next/compare/v0.4.1...v0.4.2)

This update fixes ckb-next for macOS Catalina

Note for Ubuntu PPA users: Please cancel this update.
An update will be released shortly through the PPA that also disables this prompt in the future.

Support for new devices:
- Harpoon RGB Pro
- Ironclaw RGB

Important bugfixes:

- ckb-next-daemon now correctly requests permission on macOS Catalina
- Music visualiser is now included again in macOS packages
- GUI no longer crashes if an animation has no keys set

Notes:

- Included quazip was updated to fix deprecation warnings

## [v0.4.1](https://github.com/ckb-next/ckb-next/tree/v0.4.1) (2019-08-27)
[Full Changelog](https://github.com/ckb-next/ckb-next/compare/v0.4.0...v0.4.1)

Important bugfixes:

- Specified keyboard layout is no longer being reverted on restart
- Pipe animation no longer quits if it can't read data
- Media keys work again on the legacy K65
- Daemon no longer crashes on quit on macOS
- Devices are now re-activated after resume on Linux
- Gradient animation no longer flickers with dithering enabled
- HiDPI has been disabled by default due to screen resolution misdetection
- Max DPI is now per-device
- Extra words are now filtered from the device name
- gpg2 is preferred over gpg for signature verification

New features:

- An uninstall button has been added for macOS users that allows complete uninstallation, including older versions
- Life animation has a new transparent preset
- Confirmation dialog boxes have been added when deleting profiles, modes, and animations
- Animation preview now gets temporarily disabled while ckb-next is out of focus
- Support for multiple animation paths has been added
- Pollrate can now be changed through the GUI

Notes for packagers:

- Udev rules are now installed by default in /lib/udev/rules.d.
- Udev rule installation path can be changed with UDEV_RULE_DIRECTORY in cmake
- Init systems can manually be force-enabled with FORCE_INIT_SYSTEM in cmake

## [v0.4.0](https://github.com/ckb-next/ckb-next/tree/v0.4.0) (2019-03-09)
[Full Changelog](https://github.com/ckb-next/ckb-next/compare/v0.3.2...v0.4.0)

NOTICE FOR MAC USERS: ckb-next 0.4.0 stores its data using a different backend. If you upgrade, your settings and profiles will be migrated. In case of a downgrade, any changes will not propagate to newer versions of the software.

Support for new devices:
- K70 RGB MK.2 LP
- M65 RGB Elite
- M95 Legacy

Important bugfixes:
- ST100 now works on macOS properly
- Strafe RGB MK.2 logo is mapped to the right LEDs, and media buttons are positioned correctly
- A buffer overflow causing the daemon to crash or hang has been fixed
- quickinstall can now run correctly with multibyte characters in the path
- Rename now works correctly in the Profile Manager context menu
- Animations with no presets no longer get loaded to prevent crashes
- K70 Lux is now marked correctly as monochrome
- ckb-next can be built again with Qt 5.2
- Save to hardware now gets disabled in the context menu for unsupported devices
- Settings backend has been changed to ini for macOS, preventing silent configuration data corruption
- The K55, the ST100 and the Polaris now have working default profiles
- A few cases that may cause the GUI to crash have been found and fixed
- Blank animation names are no longer allowed

New features:
- GUI has gained a monochrome tray icon option (QSystemTrayIcon only)
- Device brightness can be controlled by scrolling on the tray icon on Linux systems that support it (AppIndicator or KDE)
- Profiles and modes can now be changed with the --profile and --mode arguments passed to the GUI binary
- New Conway's Game of Life animation
- New 'snake' animation
- New 'pipe' animation, allowing the user to send rgb data from external applications
- Music Visualizer (mviz) is now supported on macOS, and has been reworked to function correctly
- A --version argument has been added to the daemon

Notes:
- On Linux, the udev rule has been renamed and moved
- On Linux, for builds with libappindicator, it can be force enabled/disabled by setting the CKB_NEXT_USE_APPINDICATOR environment variable
- On Linux, libappindicator support is also enabled if the Qt platform theme is gtk2 (Useful for Unity on Ubuntu 19.04)
- The restart command has been removed from the daemon
- Threads are now named for easier debugging
- Signal handler has been rewritten
- A few unneeded packets are no longer being sent to M65 mice
- Only Red channel data is being sent to monochrome devices

## [v0.3.2](https://github.com/ckb-next/ckb-next/tree/v0.3.2) (2018-10-07)
[Full Changelog](https://github.com/ckb-next/ckb-next/compare/v0.3.1...v0.3.2)

Important news:

- This version fixes a major bug for the Scimitar Pro, causing the daemon to not properly interface with the mouse. This could require manually reflashing the mouse firmware if a firmware update was performed with 0.3.1 or earlier.
- An updater has been created to more quickly push bugfixes to users, especially for those on macOS.
- There is a new Space Invaders-like minigame that can be played as an animation, thanks to @mvladimirovich.

Support for new devices:

- Strafe RGB MK.2; by accidental omission from the previous release.
- K66

Important bugfixes:

- The Scimitar Pro now defaults to v2.xx endpoints
- The DPI LED on mice now changes in sniper mode
- The GUI will warn if uinput cannot be loaded on Linux
- The "Save to Hardware" button on the Scimitar and Glaive has been greyed out due to a current lack of support
- Thanks to the efforts of @Kedstar99, the codebase compiles with much fewer warnings
- The daemon will retry talking to the Karabiner kext in case it runs before the kext is ready
- Binding commands to wheels will now repeat properly
- QuaZip is no longer required for daemon-only builds
- KissFFT is now built as a static library
- Symbolic links are now created in /dev/input/by-id for evdev users
- Fixed a regression for devices using the legacy protocol
- The forwards and backwards keys are now bound by default on Mac
- Workaround for using the Polaris on macOS

Note for packagers:

- If ckb-next is updated through a package management system, `-DDISABLE_UPDATER=1` should be passed to CMake

## [v0.3.1](https://github.com/ckb-next/ckb-next/tree/v0.3.1) (2018-07-31)
[Full Changelog](https://github.com/ckb-next/ckb-next/compare/v0.3.0...v0.3.1)

Important news:

- macOS now uses Karabiner Elements as a backend. You will need to enable the kext for ckb-next to function.

Support for new devices:

- K70 RGB MK.2
- K70 RGB MK.2 SE

New major features:

- macOS 10.14 Mojave support
- Karabiner Elements is used as an input backend
- The GUI now has a generate ckb-next-dev-detect report button

Important bugfixes:

- Rapidfire keyboards have a workaround on shutdown
- Bugfix for independent X/Y DPI
- The new Strafe NRGB has firmware updates now
- The GUI pops up a warning when trying to bind the Windows key with Winlock enabled
- The GUI now has a DPI indicator
- The K68 NRGB winlock light now lights up
- The GUI handles SIGINT/SIGTERM cleanly, avoiding stalled devices
- Strafe sidelights now save state on GUI start
- Original Strafe NRGB's sidelights now toggle correctly
- ckb-next-dev-detect is now installed to the system and added to PATH

## [v0.3.0](https://github.com/ckb-next/ckb-next/tree/v0.3.0) (2018-04-30)
[Full Changelog](https://github.com/ckb-next/ckb-next/compare/v0.2.9...v0.3.0)

Important news:

- Binaries have been renamed from ckb to ckb-next and qmake has been replaced with CMake

Support for new devices:

- K55 RGB
- K68 RGB
- K90 Legacy
- Katar RGB
- Polaris RGB
- ST100 RGB

New major features:

- Use URB Interrupts to read data from devices
- Profile Import/Export
- Keyboard Layout autodetection
- CMake build system
- Relocation of binaries
- No single global layout for all devices in the GUI

Important bugfixes:

- Deadlock on daemon exit
- Support for K95 Platinum profile switch key
- Keymap corruption on the GUI
- Pulseaudio GUI deadlock
- macOS Sierra and higher mouse fixes
- Use udev to detect the appropriate endpoint max packet size
- Ignore devices in BIOS mode
- Disable save to hardware for unsupported devices
- Keymap patches for K68
- Devices not being enabled on resume on macOS
- Workaround for linux kernel out-of-bounds write
- Memory leaks on firmware update

## [v0.2.9](https://github.com/ckb-next/ckb-next/tree/v0.2.9) (2018-01-21)
[Full Changelog](https://github.com/ckb-next/ckb-next/compare/v0.2.8...v0.2.9)

Important news:
* Project was moved to the organization since mattanger has disappeared
* Binaries will be renamed in `v0.3.0`

Important changes:
* Significantly lower CPU usage on idle
* Add support for K95 Platinum
* Add support for K68
* Add support for (new) Strafe non-RGB
* Add support for Glaive
* Add support for Harpoon
* Add support for Corsair firmware v3
* Add support for pt\_br layout
* Add support for Japanese layout
* Add macro delays to the GUI
* New FIRMWARE file structure
* GUI warning when daemon is not running
* udev rule to remove joystick tag
* Dynamic keymap patching
* Require Qt >=5.2
* Numerous small fixes and improvements

## [v0.2.8](https://github.com/mattanger/ckb-next/tree/v0.2.8) (2017-05-06)
[Full Changelog](https://github.com/mattanger/ckb-next/compare/v0.2.7...v0.2.8)

- `ckb` is now `ckb-next`, __but the binaries will be renamed in `v0.2.9`__
- Releases and release notes are provided from now on [\#10](https://github.com/mattanger/ckb-next/issues/10) [\#66](https://github.com/mattanger/ckb-next/issues/66)
- Numerous PRs from `ckb` merged [\#4](https://github.com/mattanger/ckb-next/issues/4)
- SIGSEGV when deleting copied profile is fixed [\#38](https://github.com/mattanger/ckb-next/issues/38)
- Modifier keys not working on macOS Sierra are temporarily fixed [\#29](https://github.com/mattanger/ckb-next/issues/29)
- Compilation errors on macOS Sierra are fixed [\#134](https://github.com/mattanger/ckb-next/issues/134)
- `make debug` target on Linux is fixed [\#79](https://github.com/mattanger/ckb-next/issues/79)
- Heat map animation issues fixed [\#30](https://github.com/mattanger/ckb-next/issues/30)
- Compilation of the music visualizer fixed [\#21](https://github.com/mattanger/ckb-next/issues/21)
- Hardware profile loading error on firmware 2.05 fixed [\#24](https://github.com/mattanger/ckb-next/pull/24)
- Project's own firmware table created, signed and populated with new devices [\#60](https://github.com/mattanger/ckb-next/pull/60)
- Arch Linux and Fedora/CentOS packages added [\#80](https://github.com/mattanger/ckb-next/pull/80) [\#5](https://github.com/mattanger/ckb-next/issues/5) [\#41](https://github.com/mattanger/ckb-next/pull/41)
