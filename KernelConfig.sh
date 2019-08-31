#!/bin/bash
cd ./linux
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- menuconfig
