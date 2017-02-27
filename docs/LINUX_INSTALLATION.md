# Linux Installation

There are three ways to install this software: automatic, semi-automatic and manual methods.

### Automatic method

"Automatic" method that means you install a pre-made package by a package maintainer, thus trusting him and the repository this package comes from. *ckb-next* developers cannot guarantee their operability and integrity. If you encounter any issue with the packages below, contact the maintainer on the package's hosting website before opening an issue. However, the project is always ready to provide a relevant space for further discussions. Moreover, *if you want to maintain a package for your platform, contact us by opening a new issue*. The general discussion on package maintaining is [here](https://github.com/mattanger/ckb-next/issues/5).

* **Arch Linux** *(maintained by [@makz27](https://github.com/makz27), [@light2yellow](https://github.com/light2yellow)):*
	* [`aur/ckb-next-git`](https://aur.archlinux.org/packages/ckb-next-git) - based on `master` branch
	* [`aur/ckb-next-latest-git`](https://aur.archlinux.org/packages/ckb-next-latest-git) - based on `testing` branch
* **Fedora 24/25, CentOS/RHEL 7** *(maintained by [@hevanaa](https://github.com/hevanaa)):*
    * [`johanh/ckb`](https://copr.fedorainfracloud.org/coprs/johanh/ckb/) - based on `master` branch

### Semi-automatic method

#### Preparation:

ckb-next requires Qt5 (Qt 5.6 recommened for OS X), libudev, zlib, gcc, g++, and glibc.

* Ubuntu: `sudo apt-get install build-essential libudev-dev qt5-default zlib1g-dev libappindicator-dev`
* Fedora: `sudo dnf install zlib-devel qt5-qtbase-devel libgudev-devel libappindicator-devel systemd-devel gcc-c++`
* Arch: `sudo pacman -S base-devel qt5-base zlib`
* Other distros: Look for `qt5` or `libqt5*-devel`

Note: If you build your own kernels, ckb-next requires the uinput flag to be enabled. It is located in `Device Drivers -> Input Device Support -> Miscellaneous devices -> User level driver support`. If you don't know what this means, you can ignore this.

#### Installing:

You can download ckb-next using the "Download zip" option on the right. Extract it and open the ckb-master directory. The easiest way to install ckb is to double-click the `quickinstall` script and run it in a Terminal. It will attempt to build ckb and then ask if you'd like to install/run the application. If the build doesn't succeed, or if you'd like to compile ckb manually, see [`BUILD.md`](https://github.com/ccMSC/ckb/blob/master/BUILD.md) for instructions.

#### Upgrading:

To install a new version of ckb, or to reinstall the same version, first delete the ckb-master directory and the zip file from your previous download. Then download the source code again and re-run `quickinstall`. The script will automatically replace the previous installation. You may need to reboot afterward.

#### Uninstalling:

First, stop the ckb-daemon service and remove the service file.
* If you have systemd (Ubuntu versions starting with 15.04):
```
sudo systemctl stop ckb-next-daemon
sudo rm -f /usr/lib/systemd/system/ckb-next-daemon.service
```
* If you have Upstart (Ubuntu versions earlier than 15.04):
```
sudo service ckb-next-daemon stop
sudo rm -f /etc/init/ckb-next-daemon.conf
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
sudo rm -f /usr/bin/ckb-next /usr/bin/ckb-next-daemon /usr/share/applications/ckb-next.desktop /usr/share/icons/hicolor/512x512/apps/ckb-next.png
sudo rm -rf /usr/bin/ckb-next-animations
```
