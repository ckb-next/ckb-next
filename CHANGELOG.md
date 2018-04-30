# Change Log

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
