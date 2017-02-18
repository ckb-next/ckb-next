#!/usr/bin/env bash
open -a 'ckb-next' --args --close
launchctl unload /Library/LaunchDaemons/ckb-next-daemon.plist
exit 0
