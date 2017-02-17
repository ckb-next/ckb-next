#!/usr/bin/env bash
launchctl load /Library/LaunchDaemons/ckb-next-daemon.plist >/dev/null 2>&1
open -a 'ckb-next' --args --background >/dev/null 2>&1
exit 0
