#!/usr/bin/env bash
echo "Stopping ckb-next app"
/usr/bin/ckb-next --close >/dev/null 2>&1
/usr/bin/ckb --close >/dev/null 2>&1

echo "Detecting system"
systemd=$(systemctl --version 2>/dev/null)
systemdc=$?
upstart=$(service --version 2>/dev/null)
upstartc=$?
openrc=$(openrc --version 2>/dev/null)
openrcc=$?
if [[ $systemdc -eq 0 && $(echo "$systemd" | wc -l) -ge 1 ]]; then
    echo "System service: systemd detected"
    echo "Stopping and removing service files"
    sudo systemctl stop ckb-daemon 2>/dev/null
    sudo systemctl disable ckb-daemon 2>/dev/null
    sudo rm -f /usr/lib/systemd/system/ckb-daemon.service 2>/dev/null
    sudo systemctl daemon-reload 2>/dev/null
    sudo systemctl stop ckb-next-daemon 2>/dev/null
    sudo systemctl disable ckb-next-daemon 2>/dev/null
    sudo rm -f /usr/lib/systemd/system/ckb-next-daemon.service 2>/dev/null
    sudo systemctl daemon-reload 2>/dev/null
elif [[ $upstartc -eq 0 && $(echo "$upstart" | wc -l) -ge 1 ]]; then
    echo "System service: Upstart detected"
    echo "Stopping and removing service files"
    sudo service ckb-daemon stop 2>/dev/null
    sudo rm -f /etc/init/ckb-daemon.conf 2>/dev/null
    sudo service ckb-next-daemon stop 2>/dev/null
    sudo rm -f /etc/init/ckb-next-daemon.conf 2>/dev/null
elif [[ $openrcc -eq 0 && $(echo "$openrc" | wc -l) -ge 1 ]]; then
    echo "System service: openrc detected"
    echo "Stopping and removing service files"
    sudo rc-service ckb-daemon stop 2>/dev/null
    sudo rc-update del ckb-daemon default 2>/dev/null
    sudo rm -f /etc/init.d/ckb-daemon 2>/dev/null
    sudo rc-service ckb-next-daemon stop 2>/dev/null
    sudo rc-update del ckb-next-daemon default 2>/dev/null
    sudo rm -f /etc/init.d/ckb-next-daemon 2>/dev/null
else
    echo "No supported system service daemons detected."
fi

echo "Removing ckb-next app and daemon"
rm -rf /opt/ckb 2>/dev/null
rm -f /usr/bin/ckb 2>/dev/null
rm -rf /opt/ckb-next 2>/dev/null
rm -f /usr/bin/ckb-next 2>/dev/null

echo "Removing support files"
rm -f /usr/share/applications/ckb.desktop 2>/dev/null
rm -f /usr/share/applications/ckb-next.desktop 2>/dev/null
rm -f /usr/share/icons/hicolor/512x512/apps/ckb.png 2>/dev/null
rm -f /usr/share/icons/hicolor/512x512/apps/ckb-next.png 2>/dev/null
