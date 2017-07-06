#!/usr/bin/env bash

echo "Stopping GUI"
ckb --close
ckb-next --close

echo "Stopping and removing service file"

echo "Detecting init system"
if [[ -e "/run/systemd/system" ]]; then
    echo "systemd detected"
    sudo systemctl stop ckb{,-next}-daemon
    sudo systemctl disable ckb{,-next}-daemon
    sudo rm -f /usr/lib/systemd/system/ckb{,-next}-daemon.service
    sudo systemctl daemon-reload
elif [[ -e "/run/openrc/softlevel" ]]; then
    echo "OpenRC detected"
    sudo rc-service ckb-daemon stop
    sudo rc-service ckb-next-daemon stop
    sudo rc-update del ckb-daemon default
    sudo rc-update del ckb-next-daemon default
    sudo rm -f /etc/init.d/ckb-daemon
    sudo rm -f /etc/init.d/ckb-next-daemon
elif [[ "$(initctl --version)" =~ "upstart" ]]; then
    echo "Upstart detected"
    sudo service ckb-daemon stop
    sudo service ckb-next-daemon stop
    sudo rm -f /etc/init/ckb{,-next}-daemon.conf
    sudo initctl reload-configuration
else
    echo "No supported init system detected. Skipping..."
fi

echo "Removing GUI and daemon"
sudo rm -rf /opt/ckb{,-next}
sudo rm -f /usr/bin/ckb{,-next}
sudo rm -f /usr/bin/ckb{,-next}-daemon

echo "Removing support files"
sudo rm -rf /usr/bin/ckb{,-next}-animations
sudo rm -f /usr/share/applications/ckb{,-next}.desktop
sudo rm -f /usr/share/icons/hicolor/512x512/apps/ckb{,-next}.png

echo "Success!"
