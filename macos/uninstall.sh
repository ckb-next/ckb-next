#!/usr/bin/env bash

echo "ckb-next uninstaller"

echo "Stopping GUI"
open -a 'ckb' --args --close
open -a 'ckb-next' --args --close

echo "Stopping the daemon and removing service file"
echo "Please enter your password"
sudo launchctl unload /Library/LaunchDaemons/ckb-next-daemon.plist
sudo launchctl unload /Library/LaunchDaemons/com.ckb.daemon.plist
sudo launchctl unload /Library/LaunchDaemons/org.ckb-next.daemon.plist
sudo rm -f /Library/LaunchDaemons/ckb-next-daemon.plist
sudo rm -f /Library/LaunchDaemons/com.ckb.daemon.plist
sudo rm -f /Library/LaunchDaemons/org.ckb-next.daemon.plist
sudo rm -f /Library/LaunchAgents/org.ckb-next.daemon_agent.plist

echo "Removing GUI, daemon and support files"
sudo rm -rf /Applications/ckb{,-next}.app
sudo rm "/Library/Application Support/ckb-next-daemon"
echo "Finished!"
