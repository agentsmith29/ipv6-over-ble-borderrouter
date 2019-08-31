#!/usr/bin/env bash

#!/bin/bash

DAEMON_HOME="/home/pi/ipv6-over-ble-borderrouter/BLEd"

# on RPi you need

while getopts blcad option
do
  case "${option}"
  in
    b) BUILD_BLUETOOTH=true;;
    l) BUILD_LIBCONFIG=true;;
    c) CLEAN=true;; # If we want to reset the hci device
    a) BUILD_ALL=tr ue;;
    d) DEP=true;;
  esac
done

build_libconfig() {
    echo "***** Building libconfig *****"
    cd $DAEMON_HOME/libconfig
    make clean
    autoreconf
    autoconf
    make -j4

    echo "***** DONE *****"
}

build_bluetooth() {

    echo "***** Building bluetooth lib *****"
    if [ $DEP ]; then
        sudo apt-get install -y dbus \
            libdbus-1-dev\
            libudev-dev \
            libical-dev \
            libreadline-dev\
            libglib2.0-dev
    fi;


    cd $DAEMON_HOME/bluez
    make clean
    ./configure -enable-library
    make -j4
    echo "***** DONE *****"
}

if [ $BUILD_LIBCONFIG ]; then
    build_libconfig
fi

if [ $BUILD_BLUETOOTH ]; then
    build_bluetooth
fi



cd $DAEMON_HOME

if [ $CLEAN ]; then
    build_libconfig
    cd $DAEMON_HOME
    rm -r build
    mkdir build

fi


if [ $BUILD_ALL ]; then
    cd $DAEMON_HOME
    rm -r build
    mkdir build
    build_libconfig
    build_bluetooth
fi

cd $DAEMON_HOME
# Check if folder exists
if [ -d "build" ]; then
  cd build
else
  mkdir build
  cd build
fi

ls -l
cmake ..
make -j4
cd $DAEMON_HOME
