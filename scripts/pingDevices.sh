#!/bin/bash

echo "connect EC:A1:6C:F6:17:E0 2" > /sys/kernel/debug/bluetooth/6lowpan_control
sleep 2
echo "connect FE:8D:E4:E3:19:69 2" > /sys/kernel/debug/bluetooth/6lowpan_control


tmux \
  new-session "ping6 -I bt0 ff02::1 ; read" \; \
  split-window "watch hcitool con ; read" \; \
  split-window "ifconfig ; read" \; \
  select-layout tiled

#tmux split-window -h "ifconfig"
