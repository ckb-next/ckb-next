#!/usr/bin/env bash

echo "ckb-next restart daemon"

echo "Please enter your password"
sudo launchctl unload /Library/LaunchDaemons/org.ckb-next.daemon.plist
sudo launchctl load -w /Library/LaunchDaemons/org.ckb-next.daemon.plist
echo "Finished!"
