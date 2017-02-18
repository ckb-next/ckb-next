#!/usr/bin/env bash
echo "Stopping ckb-next app"
open -a 'ckb-next' --args --close
open -a 'ckb' --args --close >/dev/null 2>&1
echo "Stopping daemon"
sudo launchctl unload /Library/LaunchDaemons/ckb-next-daemon.plist
sudo launchctl unload /Library/LaunchDaemons/com.ckb.daemon.plist >/dev/null 2>&1
echo "Removing ckb-next app"
sudo rm -rf /Applications/ckb-next.app
sudo rm -rf /Applications/ckb.app >/dev/null 2>&1
echo "Removing daemon's plist"
sudo rm -rf /Library/LaunchDaemons/ckb-next-daemon.plist
sudo rm -rf /Library/LaunchDaemons/com.ckb.daemon.plist >/dev/null 2>&1
