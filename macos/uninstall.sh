#!/usr/bin/env bash
echo "Stopping ckb-next app"
/Applications/ckb-next.app/Contents/MacOS/ckb-next --close >/dev/null 2>&1
echo "Removing ckb-next app"
sudo rm -rf /Applications/ckb-next.app >/dev/null 2>&1
echo "Stopping daemon"
sudo launchctl unload /Library/LaunchDaemons/ckb-next-daemon.plist >/dev/null 2>&1
echo "Removing daemon's plist"
sudo rm -rf /Library/LaunchDaemons/ckb-next-daemon.plist >/dev/null 2>&1
