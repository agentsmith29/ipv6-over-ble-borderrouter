# BLEd
A scalable deamon for managing Bluetooth Low Energy Connections

# Installing
TODO: Make the installations scripts universal.

For easy installing just run the *buildRemote.sh*-Script on your local computer.
Make sure you added the correct ssh-keys and adapted the IP-Address of your Rapsberry Pi.

Adapt the line 3 in the bash-file *buildRemote.sh*, so the correct *IP-Address* will be used.
Change the line 
```RPI_IP_ADDRESS="<IP-Address>"```
to your IP-Address.

Now you are read to call the insallation script:
```bash
$ ./installRemote.sh
```
If this is your first time compiling the BLEd-Daemon you may need to compile the BlueZ library first.
In this case call
```bash
$ ./installRemote.sh -a
```
This script uses *rsync*. Probably you need to install *rsync* first. On Debian-based distros call
```bash
$ sudo apt install rsync
```
The daemon can be stared with

```bash
$ sudo systemctl start BLEd.service
```
on the Raspberry Pi.

# Using
When the deamon start up it will automatically connect the devices to the device *hci0*

For interfacing you can use the socket connection in port *11111*. You can either use telnet or netcat

```
netcat <ip-addr> 11111
```

When connected the deamon will repond with a message similar to this:
```
BLEd Daemon v 0.6.10(beta)
BLEd $ 

```
where you can enter the commands for managing the daemon.

## Commands
### remap
The remap command remaps nodes. With this you can assign a node from an hci device to another hci device. 
Syntax: remap \<bd-address\> \<from-hci-device\> \<to-hci-device\>
```
BLEd $ remap <bd-addr> 0 1
```