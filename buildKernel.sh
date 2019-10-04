#!/bin/bash
cd ./linux_4_19

#make clean

#For Pi 2, Pi 3, Pi 3+, or Compute Module 3:
KERNEL=kernel7
time make -j8 ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- bcm2709_defconfig

#Then, for both:
time make -j8 ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- zImage modules dtbs
