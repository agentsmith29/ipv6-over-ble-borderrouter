#!/bin/bash

binary_dir="/home/pi/ipv6-over-ble-borderrouter/BLEd/build/BLEd"
SYSTEMD_UNIT_DIR="/etc/systemd/system/"
#mkdir ${SYSTEMD_UNIT_DIR}

#JUST FOR LOCAL TESTING
#cp cmake-build-debug/bin/6lowpan_helper /usr/bin/
if [ -d "/usr/bin/BLEd" ]; then
    rm /usr/bin/BLEd
fi
cp $binary_dir /usr/bin/


cp build/bin/BLEd /usr/bin/
cp configs/bled_config.conf /usr/bin/
cp BLEd.service ${SYSTEMD_UNIT_DIR}
echo "Installed!"
