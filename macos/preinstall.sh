#!/usr/bin/env bash
open -a 'ckb-next' --args --close >/dev/null 2>&1
launchctl unload /Library/LaunchDaemons/ckb-next-daemon.plist >/dev/null 2>&1
exit 0
