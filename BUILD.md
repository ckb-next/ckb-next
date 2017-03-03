Building ckb-next
============

Linux
-----

You can build the project by running `./qmake-auto && make` in a Terminal inside the ckb-next-master directory. The binaries will be placed in a new `bin` directory assuming they compile successfully. If you get a `No suitable qmake found` error, make sure Qt5 is installed and up to date. You may have to invoke qmake manually, then run `make` on its own. If you have Qt Creator installed, you can open `ckb-next.pro` (when asked to configure the project, make sure "Desktop" is checked) and use `Build > Build Project "ckb-next" (Ctrl+B)` to build the application instead.

#### Running as a service:

First copy the binary and the service files to their system directories:

* Upstart (Ubuntu, prior to 15.04): `sudo cp -R bin/* /usr/bin && sudo cp service/upstart/ckb-next-daemon.conf /etc/init`
* Systemd (Ubuntu 15.04 and later): `sudo cp -R bin/* /usr/bin && sudo cp service/systemd/ckb-next-daemon.service /usr/lib/systemd/system`

To launch the driver and enable it at start-up:

* Upstart: `sudo service ckb-next-daemon start`
* Systemd: `sudo systemctl start ckb-next-daemon && sudo systemctl enable ckb-next-daemon`

Open the `bin` directory and double-click on `ckb-next` to launch the user interface. If you want to run it at login, add `ckb-next --background` to your Startup Applications.

#### Running manually:

Open the `bin` directory in a Terminal and run `sudo ./ckb-next-daemon` to start the driver. To start the user interface, run `./ckb-next`. Running the driver manually may be useful for testing/debugging purposes, but you must leave the terminal window open and you'll have to re-run it at every reboot, so installing it as a service is the best long-term solution.

OSX
---

Open ckb-next.pro in Qt Creator. You should be prompted to configure the project (make sure the "Desktop" configuration is selected and not iOS). Once it's finished loading, press `Cmd+B` or select `Build > Build Project "ckb-next"` from the menu bar. When it's done, you should see a newly-created `ckb-next.app` in the project directory. Exit Qt Creator.

Alternatively, open a Terminal in the ckb-next-master directory and run `./qmake-auto && make`. It will detect Qt automatically if you installed it to one of the standard locations. You should see a newly created `ckb-next.app` if the build is successful.

#### Running as a service:

Copy `ckb-next.app` to your Applications folder. Copy the file  [`service/launchd/com.ckb-next.daemon.plist`](https://raw.githubusercontent.com/ccMSC/ckb-next/master/service/launchd/com.ckb-next.daemon.plist) to your computer's `/Library/LaunchDaemons` folder (you can get to it by pressing `Cmd+Shift+G` in Finder and typing the location). Then open a Terminal and run the following commands to launch the driver:

```
sudo chown root:wheel /Library/LaunchDaemons/com.ckb-next.daemon.plist
sudo chmod 0700 /Library/LaunchDaemons/com.ckb-next.daemon.plist
sudo launchctl load /Library/LaunchDaemons/com.ckb-next.daemon.plist
```

After you're done, open `ckb-next.app` to launch the user interface.

#### Running manually:

Open a Terminal in the ckb-next directory and run `sudo ckb-next.app/Contents/Resources/ckb-next-daemon` to start the driver. Open `ckb-next.app` to start the user interface. Note that you must leave the terminal window open and must re-launch the driver at every boot if you choose this; installing as a service is the better long term solution.
