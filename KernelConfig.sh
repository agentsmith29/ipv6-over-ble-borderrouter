#!/bin/bash
cd ./linux_4_19
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- menuconfig
