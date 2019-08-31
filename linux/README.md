# Linux kernel - bluetooth_6lowpan
============
This is a documentation of all the applied changes to the bluetooth_6lowpan kernel module.


## Changes
### Issue 1 - Make multiple connected devices pingable.
#### Description
In the default version of *6lowpan.c* if more than one node is connected none of then are pinagble

#### Solution
This fix has been found on: https://www.spinics.net/lists/linux-bluetooth/msg72040.html
"If there are more than one node connected and there isn't any route
information attempt to check if destination address matches any of the
link-local peer addresses."

In the *6lowpan.c* source file the following lines (213) in fuction ```lowpan_peer *peer_lookup_dst``` had
been changed.
Instead of returning *NULL*, the *in6_addr struct* ```nexthop``` is assigned to ```daddr```.
```c
if (ipv6_addr_any(nexthop)) {
   // return NULL;
   nexthop = daddr; // Fix for issue 1
}
```


### Issue 2 - Select HCI-Device
#### Description
If more than one hci-device is present you can't specify which device should be used.

#### Solution
This solution contains an implementation of the function ```set_default_hci(bdaddr_t *def_hci_address)```
in the *hci_con.c* (net/bleutooth/hci_con.c, 624).When a *select* command is sent to the *6lowpan_control* file, 
the ```set_default_hci``` function is called and the value of ```uint8_t set_default_hci_device[8]``` will be set to the given hci address.
So, by calling this function a default hci-address in *hci_con.c* will be set.
If any availabe hci-device matches with the default one (while iterating over all available hci-devices in function 
```struct hci_dev *hci_get_route(...)```) this hci-device will be used, otherwise the last one found.

#### Example
A new command for the *6lowpan_control* (under /proc/ble/) has been implemented.
With the newly implemented *select*-command a new default hci-device can be selected.


We will select the hci device by it's *BD Address* where addr is the BD address 
of the desired hci device:
```bash
$ echo "select <addr>" > /proc/ble/6lowpan_control
```
The BD Address of all avaiable hci devices can be found with the command:
```$ hciconfig```
An possible output could be
```
hci2:   Type: Primary  Bus: UART
        BD Address: B8:27:EB:40:9B:06  ACL MTU: 1021:8  SCO MTU: 64:1
        UP RUNNING
        RX bytes:1614 acl:33 sco:0 events:76 errors:0
        TX bytes:3041 acl:42 sco:0 commands:47 errors:0

hci1:   Type: Primary  Bus: USB
        BD Address: 5C:F3:70:8D:60:47  ACL MTU: 1021:8  SCO MTU: 64:1
        UP RUNNING
        RX bytes:950 acl:0 sco:0 events:46 errors:0
        TX bytes:1702 acl:0 sco:0 commands:46 errors:0

hci0:   Type: Primary  Bus: USB
        BD Address: 00:1A:7D:DA:71:13  ACL MTU: 310:10  SCO MTU: 64:8
        UP RUNNING
        RX bytes:1669 acl:34 sco:0 events:101 errors:0
        TX bytes:3752 acl:43 sco:0 commands:54 errors:0

```
For selecting e. g. *hci1* just enter
```bash
$ echo "select 5C:F3:70:8D:60:47" > /proc/ble/6lowpan_control
```
Afterwards any ble node can be connected via 
```bash
$ echo "connect <bdaddr> 2" > /proc/ble/6lowpan_control
```
If you want to change the where a node should connect you have to change the default device via the *select*- command 
first