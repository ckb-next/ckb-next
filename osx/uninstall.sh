#!/usr/bin/env bash
echo "Stopping daemon"
sudo launchctl unload /Library/LaunchDaemons/ckb-next-daemon.plist
echo "Removing daemon's plist"
sudo rm -rf /Library/LaunchDaemons/ckb-next-daemon.plist
echo "Stopping ckb-next"
/Applications/ckb-next.app/Contents/MacOS/ckb-next --close
echo "Removing ckb-next"
sudo rm -rf /Applications/ckb-next.app
