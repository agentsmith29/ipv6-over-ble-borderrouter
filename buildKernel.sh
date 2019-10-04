#!/bin/bash
cd ./linux_4_19

# make clean

# For fixing the "command not found" error while running with sudo!
PATH="$PATH:./../linux_tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin"

#For Pi 2, Pi 3, Pi 3+, or Compute Module 3:
KERNEL=kernel7
time make -j8 ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- bcm2709_defconfig

#Then, for both:
time make -j8 ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- zImage modules dtbs
