#!/bin/sh


sudo visudo
yava ALL=(ALL) NOPASSWD:ALL

sudo apt-get install -y mc winbind libnss-winbind cifs-utils

sudo mcedit /etc/nsswitch.conf

# hosts: files dns wins
# networks: files




sudo apt-get install -y git rsync md5deep fakeroot
sudo apt-get install -y qt5-default libqt5sql5-psql make g++ libssl-dev
sudo apt-get install -y libavcodec-dev libswscale-dev libavformat-dev libavdevice-dev
sudo apt-get install -y liblivemedia-dev
# (
sudo apt-get install -y libqt5serialport5-dev
# (
sudo apt-get install -y libqt5x11extras5-dev
libqt5x11extras5

mkdir ~/Code
svn co https://X2/svn/Code ~/Code