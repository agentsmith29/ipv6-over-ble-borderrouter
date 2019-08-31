#!/bin/bash
#==============================================================================#
# Settings

TIMEOUT=5       # This will set the timeout for canning for ble devices (in s)
SEL_DEVICE=""   # This will store the selected device


while getopts t:r option
do
  case "${option}"
  in
    t) TIMEOUT=${OPTARG};;
    r) res=true;; # If we want to reset the hci device
  esac
done

# Only for printing messages and errors
Msg() {
 echo -e "[MSG] $1"
}

ERR() {
 echo -e "[ERR] $1. Will exit now."
 exit -1
}


#==============================================================================#
# For this we need to be logged in as root
# Run  'sudo su'  to log in as root.
if [ "$EUID" -ne 0 ]; then 
  Msg "This script currently don't support running from other users than root."
  ERR "Run sudo su to change to root"
fi





# Log in as a root user.
# sudo su We don not need this one
# Mount debugfs file system.


device_scan() {
  
  Msg "Scan for availabe devices (Timeout ${TIMEOUT}s)"
  # Read address of the nRF5x device.
  # From http://federicopfaffendorf.com.ar/hcitool-lescan-timeout/
  hcitool lescan > .tmp_lescan &
  pid=$!
  sleep $TIMEOUT
  kill -INT $pid
  wait $pid
  
  # This will just read the temp file and split after every \n so we can display
  # our devices as an list
  IFS=' '
  devices=$(cat .tmp_lescan)
  arr=()
  while read -r line; do
    device_add_split+=("$line")
  done <<< "$devices"  
  
  device_add_split+=("Exit")
  # Just display the found devices
  echo "Found ${#device_add_split[@]} device(s)"
  # Print the menue
  SelectDevice "${#device_add_split[@]}" "${device_add_split[@]}" 

  # Grep for the device address
  SEL_DEVICE_ADDRESS=$(echo $SEL_DEVICE | cut -c 1-17)
  echo "Selected: $SEL_DEVICE_ADDRESS" 

  # Cleanup
  SEL_DEVICE=""
  rm .tmp_lescan        
  device_add_split=()
}




SelectDevice ()
{

  arrsize=$1
  #echo "${@:2}"
  select option in "${@:2}"; do
   if [ 1 -le "$REPLY" ] && [ "$REPLY" -le $((arrsize)) ];
    then
      SEL_DEVICE=$option
      break;
    else
      echo "Incorrect Input: Select a number 1-$arrsize"
      return -1;
    fi
  done
}


# This is something we don not need to do every time this script gets called!
# We will first check if the debug fs is already mounted. Otherwise we will try
# to mount the fs.
MountDebugFs() {
  mount="/sys/kernel/debug "

  if grep -qs "$mount" /proc/mounts; then
    Msg "Debugfs file system already mounted."
  else
    Msg "Mount debugfs file system."
    mount -t debugfs none $mount 
    if [ $? -eq 0 ]; then
      Msg "Debugfs file system mounted."
    else
      ERR "Something went wrong with the mount."
    fi
  fi
}




# Load 6LoWPAN module.
Msg "Load 6LoWPAN module."
modprobe bluetooth_6lowpan > /dev/null

# Enable the bluetooth 6lowpan module.
Msg "Enable the bluetooth 6lowpan module."
# echo 1 > /sys/kernel/debug/bluetooth/6lowpan_enable
echo 1 > /proc/ble/6lowpan_enable


# This is basically something we don not need in the script. This just look for 
# available HCI devices and displays a couple of information about available 
# devices. You can uncomment this if you want to gain informations about the 
# hci devices

#<-># hciconfig > /dev/null

# We assume the devices is hci0. You can grep for the divice with the command #
# given above.
# TODO: Implement a grep command for hci devices?

# Reset HCI device - for example hci0 device.
# This si something we dont want to every time, e. g. if an device is already
# connected this would result in an disconnection of this device.
# So we need to check if devices are already connected, otherwise dont reset it.
no_conn_devices=$(hcitool con | grep "handle" | wc -l)

if [ "$no_conn_devices" = "0" ]  || [ "$res" = "true" ]; then
  # No connected devices. We can now safely reset the hci0 device.
  Msg "Resetting hci0 module."
  hciconfig hci0 reset
fi

# This command just check for connected devices and greps for 'handle'.
# The second pipe is just for counting the occurrences of the found word.
# With result this we can determine of there are already open connections
while true
do
  # Now want to perform a device scan. For this, the following function was
  # implemented.
  device_scan   # Scan for available devices
 
  if [ "$SEL_DEVICE_ADDRESS" = "Exit" ]; then
    # No connected devices. We can now safely reset the hci0 device.
    break
  fi


  # Now we just need to connect the device
  Msg "Connect to the device."
  # Somethimes the first attamp fails (suggestion from nordic) so we will retry 5 
  # times 
  for con in {0..1}
  do
    echo "$con $SEL_DEVICE_ADDRESS..."
    echo "connect $SEL_DEVICE_ADDRESS 2" > /proc/ble/6lowpan_control
  done
  sleep 2
done


#******************************************************************************#
#******************************************************************************#
# This is just a output to show, thats everything was connected properly.
hcitool con
ifconfig

exit 0;



# This are just a couple of commands and not part of this script

# Try to ping the device using its link-local address, for example, on bt0 interface.
ping6 -i bt0 fe80::2aa:bbff:fexx:yyzz

# Disconnect from the device. 
# TODO: This doesn't work form me???
echo "disconnect 00:AA:BB:XX:YY:ZZ" > /sys/kernel/debug/bluetooth/6lowpan_control
# Check if there are active connections left.

ifconfig
