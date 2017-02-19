#!/usr/bin/env bash
echo "Stopping ckb-next app"
open -a 'ckb-next' --args --close 2>/dev/null
open -a 'ckb' --args --close >/dev/null 2>&1
echo "Stopping daemon"
sudo launchctl unload /Library/LaunchDaemons/ckb-next-daemon.plist 2>/dev/null
sudo launchctl unload /Library/LaunchDaemons/com.ckb.daemon.plist >/dev/null 2>&1
echo "Removing ckb-next app and daemon"
sudo rm -rf /Applications/ckb-next.app 2>/dev/null
sudo rm -rf /Applications/ckb.app >/dev/null 2>&1
echo "Removing daemon's plist"
sudo rm -rf /Library/LaunchDaemons/ckb-next-daemon.plist 2>/dev/null
sudo rm -rf /Library/LaunchDaemons/com.ckb.daemon.plist >/dev/null 2>&1
