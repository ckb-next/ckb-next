#!/usr/bin/env bash
sudo chown -R $(whoami):staff /Applications/ckb-next.app
launchctl load /Library/LaunchDaemons/ckb-next-daemon.plist
open -a 'ckb-next'
exit 0
