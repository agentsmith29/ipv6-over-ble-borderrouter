#!/bin/bash

cd linux
KERNEL=kernel7
result1=$(lsblk | grep "sd${1}1")
result2=$(lsblk | grep "sd${1}1")

MOUNT_DIR="/tmp/mnt"
MOUNT_DIR_FAT32="$MOUNT_DIR/fat32"
MOUNT_DIR_EXT4="$MOUNT_DIR/ext4"


if [ "$result1" == "" ]; then
   echo "Error 1"
   exit -1
fi

if [ "$result2" == "" ]; then
   echo "Error 2"
   exit -1
fi



#For fixing the "command not found" error while running with sudo!
PATH="$PATH:~/workspace/IPv6-BLE-Router/linux_tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin"





mkdir $MOUNT_DIR
mkdir $MOUNT_DIR_FAT32
mkdir $MOUNT_DIR_EXT4
ls -l $MOUNT_DIR 


mount "/dev/sd${1}1" $MOUNT_DIR_FAT32
mount "/dev/sd${1}2" $MOUNT_DIR_EXT4

echo "RUNNING..."
sudo make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- INSTALL_MOD_PATH=$MOUNT_DIR_EXT4 modules_install
echo "start copying..."

cp $MOUNT_DIR_FAT32/$KERNEL.img $MOUNT_DIR_FAT32/$KERNEL-backup.img
cp arch/arm/boot/zImage $MOUNT_DIR_FAT32/$KERNEL.img
cp arch/arm/boot/dts/*.dtb $MOUNT_DIR_FAT32/
cp arch/arm/boot/dts/overlays/*.dtb* $MOUNT_DIR_FAT32/overlays/
cp arch/arm/boot/dts/overlays/README $MOUNT_DIR_FAT32/overlays/

echo "unmounting..."
umount $MOUNT_DIR_FAT32
umount $MOUNT_DIR_EXT4

read -p "Press enter to continue. Please make shure that all devices ar unmounted!"
rm -r $MOUNT_DIR
echo "done, you can now boot!"
