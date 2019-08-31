#!/usr/bin/env bash

RPI_IP_ADDRESS="192.168.8.101"

while getopts a option
do
  case "${option}"
  in
    a) BUILD_ALL=tr ue;;
  esac
done

rsync -r -t -p -v --progress -s \
--exclude=linux_tools \
--exclude=linux \
--exclude=buildKernel.sh \
--exclude=copy2sd.sh \
--exclude=KernelConfig.sh \
--exclude=daemon/bluez \
--exclude=daemon/libconfig \
--exclude=daemon/cmake-build-debug \
--exclude=daemon/buildx86 \
--exclude=BLEd/bluez \
--exclude=BLEd/libconfig/build \
--exclude=BLEd/cmake-build-debug \
--exclude=BLEd/buildx86 \
--exclude=.git \
/home/developer/workspace/ipv6-over-ble-borderrouter/ pi@${RPI_IP_ADDRESS}:/home/pi/ipv6-over-ble-borderrouter


PATH_TO_DAEMON="~/ipv6-over-ble-borderrouter/BLEd"
ssh pi@${RPI_IP_ADDRESS} "sudo systemctl stop BLEd.service"

if [ $BUILD_ALL ]; then
    ssh pi@${RPI_IP_ADDRESS} "cd $PATH_TO_DAEMON && ./buildRPi.sh -a"
else
    ssh pi@${RPI_IP_ADDRESS} "cd $PATH_TO_DAEMON && ./buildRPi.sh"
fi


ssh pi@${RPI_IP_ADDRESS} "cd $PATH_TO_DAEMON && sudo ./installDaemon.sh"
