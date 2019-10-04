# IPv6-BLE-Router
Source files for the IPv6-over-BLE Border Router

# Building
On a Raspberry Pi, first install the latest version of Raspbian.
Then boot your Pi, plug in Ethernet to give you access to the sources, and log in.

## Dependencies
First install Git and the build dependencies:

```sudo apt-get install git bc bison flex libssl-dev```

Next get the sources from: https://github.com/agentsmith29/ipv6-over-ble-borderrouter.git
```bash
git clone https://github.com/agentsmith29/ipv6-over-ble-borderrouter.git
```
Update the submodules
```bash
git submodule update --init
```

## Kernel configuration
For Raspberry Pi 2, Pi 3, Pi 3+, and Compute Module 3 default
build configuration

```bash
./kernelConfig.sh
```
A configuration page should open up now. Under  `Networking support -> Bluetooth subsystem support -> Bluetooth Low Energy (LE) features` make sure 
`Bluetooth 6LoWPAN support` is marked with an 'M'.

Example:

```bash
[*] Networking support
      <M> Bluetooth subsystem support
        [*]   Bluetooth Classic (BR/EDR) features
    <M>     RFCOMM protocol support
    [*]       RFCOMM TTY support
    <M>     BNEP protocol support
    [*]       Multicast filter
    [*]       Protocol filter support
    <M>     CMTP protocol support
    <M>     HIDP protocol support
    [*]     Bluetooth High Speed (HS) features
    [*]  Bluetooth Low Energy (LE) features 
    <M>     Bluetooth 6LoWPAN support
    [ ]   Bluetooth self testing support
    [*]   Export Bluetooth internals in debugf
   ```    
## Building

```bash
./buildKernel.sh
```

## Copying to SDCard

For copying to your sd-card invoke the script `./copy2sd.sh` with the
parameters <device>.

```bash
./copy2sd.sh <device>
```

You can get you sd-card's device name when invoking `lsblk`.

Possible output
```bash
   sdb
     sdb1
     sdb2`
```

If `sdb` is you sd-card, call the script with the following parameter

```bash
./copy2sd.sh sdb
```
