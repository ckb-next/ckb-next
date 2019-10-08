#!/usr/bin/env bash

echo "Please enter your password to restart ckb-next-daemon"
sudo launchctl unload /Library/LaunchDaemons/org.ckb-next.daemon.plist
sudo launchctl load -w /Library/LaunchDaemons/org.ckb-next.daemon.plist
echo "Finished!"
