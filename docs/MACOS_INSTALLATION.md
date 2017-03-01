## Linux Installation

There are three basic ways how to install this software: automatic, semi-automatic and manual methods.

### Automatic method

"Automatic" method means you run a macOS installer (in `.pkg` format) prepared by developers of this project and it does everything required.

##### Installing

Official binary:

1. Download the latest `.pkg` file from [Releases](https://github.com/mattanger/ckb-next/releases) page
2. Double-click and follow the on-screen instructions

Then just launch `ckb-next` like you would normally do with any app.

*If you would like to maintain a package for Homebrew, we are happy to accept it, use [this issue](https://github.com/mattanger/ckb-next/issues/5) for communication*.

##### Uninstalling

The Mac way:

1. Remove ckb-next from `Applications`
2. `sudo rm -f /Library/LaunchDaemons/ckb-next-daemon.plist` - removes daemon's plist

##### Upgrading

Here's something you'd not like to hear: you can't really follow project's releases without "Watching" the whole thing. [At least we are not alone.](https://github.com/isaacs/github/issues/410) Nevertheless, press "Watch" and give it a try!

*However, as soon as somebody starts maintaining this on Homebrew, you can smoothly upgrade ckb-next with the package manager.*

Anyway, if you managed to get a newer `.pkg`, just repeat what [Installing](MACOS_INSTALLATION.md#installing) describes.


### Semi-automatic method

"Semi-automatic" means that you can still get the job done, but change some of the decisions made by the developers. On the other hand, it takes some time to download all the tools.

##### Preparation for manual compilation:

ckb-next **requires** Qt5, Xcode Command Line Tools and Xcode itself.

* download Xcode from the App Store
* install Command Line Tools using (guess what) the command line: `xcode-select --install`
* install Qt5
    * either using Homebrew: `brew install qt5` aka *the recommended way*
    * or with an official Qt5 binary installer: [here](https://www.qt.io/download-open-source/) aka *the not recommended way*

If you decide to install Qt5 the second way you will need to set `BREW_QT5` to `OFF` and populate `CMAKE_PREFIX_PATH` with a root directory of Qt5 installation. Look at [CMake install options](CMAKE_CONFIG.md) to find out how.

##### Building and installing

Step-by-step:

* *clone* the repository (don't just download the zip! we need to have git information)
* `cd` into the source directory
* run
	1. `mkdir build && cd build` - required for CMake's out-of-source build
	2. `cmake ..` - generates CMake configuration files and Makefiles. Here you can pass different options and customize your installation. See [CMake install options](CMAKE_CONFIG.md) for the full list of options.
	3. `make -j"$(getconf _NPROCESSORS_ONLN)"` - compiles the code using all cores
	4. `sudo make install`
	5. as the comment says at the end of the installation, 
	```
	sudo chown -R $(whoami):staff /Applications/ckb-next.app
	```

##### Upgrading

To install a new version remove the old source code directory and continue with [Building](LINUX_INSTALLATION.md#building) (starting from cloning again). CMake will replace everything for you. You may need to reboot afterwards.

##### Uninstalling

The repository contains a script. Run `sudo macos/uninstall.sh` provided that you are in the ckb-next source code root directory.

