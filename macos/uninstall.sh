#!/usr/bin/env bash

echo "Stopping GUI"
open -a 'ckb' --args --close
open -a 'ckb-next' --args --close

echo "Stopping the daemon and removing service file"
sudo launchctl unload /Library/LaunchDaemons/ckb-next-daemon.plist
sudo launchctl unload /Library/LaunchDaemons/com.ckb.daemon.plist
sudo rm -f /Library/LaunchDaemons/ckb-next-daemon.plist
sudo rm -f /Library/LaunchDaemons/com.ckb.daemon.plist

echo "Removing GUI, daemon and support files"
sudo rm -rf /Applications/ckb{,-next}.app
