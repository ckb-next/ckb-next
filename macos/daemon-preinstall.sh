#!/usr/bin/env bash
# old plist
sudo launchctl unload -w /Library/LaunchDaemons/com.ckb.daemon.plist
# new plist
sudo launchctl unload -w /Library/LaunchDaemons/org.ckb-next.daemon.plist
exit 0
