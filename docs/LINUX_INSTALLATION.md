## Linux Installation

There are three basic ways how to install this software: automatic, semi-automatic and manual methods.


### Automatic method

"Automatic" method means you install a pre-made package by a package maintainer, thus trusting him and the repository this package comes from. *ckb-next* developers cannot guarantee the operability and integrity of these packages. If you encounter any issue with the packages below, contact the maintainer on the package's hosting website before opening an issue. However, the project is always ready to provide a relevant space for further discussions. Moreover, *if you would like to maintain a package for your platform, we are happy to accept it, use [this issue](https://github.com/mattanger/ckb-next/issues/5) for communication*.

##### Installing

* **Arch Linux** (maintained by [@makz27](https://github.com/makz27), [@light2yellow](https://github.com/light2yellow)):
	* [`aur/ckb-next-git`](https://aur.archlinux.org/packages/ckb-next-git) - based on `master` branch
	* [`aur/ckb-next-latest-git`](https://aur.archlinux.org/packages/ckb-next-latest-git) - based on `testing` branch
* **Fedora 24/25, CentOS/RHEL 7** (maintained by [@hevanaa](https://github.com/hevanaa)):
    * [`johanh/ckb`](https://copr.fedorainfracloud.org/coprs/johanh/ckb/) - based on `master` branch

##### Uninstalling

Use a package manager to uninstall ckb-next.

##### Upgrading

Use a package manager to upgrade ckb-next.


### Semi-automatic method

"Semi-automatic" means that you can still get the job easily done, but change some of the decisions made by a package maintainer.

##### Preparation for manual compilation:

ckb-next **requires** Qt5, libudev, zlib, gcc, g++, and glibc.

ckb-next has an **optional** dependency - PulseAudio API - to build a music visualizer. However, it will not be compiled by default. See [CMake install options](CMAKE_CONFIG.md) to change this.

For example:

* Ubuntu: `sudo apt-get install build-essential libudev-dev qt5-default zlib1g-dev libappindicator-dev` optionally `libpulse-dev`
* Fedora: `sudo dnf install zlib-devel qt5-qtbase-devel libgudev-devel libappindicator-devel systemd-devel gcc-c++` optionally `libpulse-devel`
* Arch Linux: `sudo pacman -S base-devel qt5-base zlib` optionally `libpulse`

Note: If you build your own kernels, ckb-next requires the `uinput` flag to be enabled. It is located in `Device Drivers -> Input Device Support -> Miscellaneous devices -> User level driver support`. If you don't know what this means, you can ignore this.

##### Building and installing

Step-by-step:

* *clone* the repository (don't just download the zip! we need to have git information)
* `cd` into the source directory
* run
	1. `mkdir build && cd build` - required for CMake's out-of-source build
	2. `cmake ..` - generates CMake configuration files and Makefiles. Here you can pass different options and customize your installation. See [CMake install options](CMAKE_CONFIG.md) for the full list of options.
	3. `make -j"$(nproc --all)"` - compiles the code using all cores
	4. `sudo make install`

##### Upgrading

To install a new version remove the old source code directory and continue with [Building](LINUX_INSTALLATION.md#building) (starting from cloning again). CMake will replace everything for you. You may need to reboot afterwards.

##### Uninstalling

The repository contains a script. Run `sudo linux/uninstall.sh` provided that you are in the ckb-next source code root directory.


### Manual method

"Manual" method means you're on your own. Do what you want and how you want. Not recommended for users not familiar with the project.

You are highly encouraged to take a look at `CMakeLists.txt` files before trying to install/remove something manually.

##### Preparation, building and installing:

Same as [Semi-automatic/Preparation, building and installing](LINUX_INSTALLATION.md#preparation) (or your own version of truth).

##### Upgrading:

Same as [Semi-automatic/Upgrading](LINUX_INSTALLATION.md#upgrading-1) (or your own version of truth).

##### Uninstalling:

You can use `uninstall.sh` mentioned in [Semi-automatic/Uninstalling](#uninstalling-2) (or your own version of truth).
