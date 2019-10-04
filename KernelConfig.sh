#!/bin/bash

cd ./linux_4_19

# For fixing the "command not found" error while running with sudo!
PATH="$PATH:./../linux_tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin"


make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- menuconfig
