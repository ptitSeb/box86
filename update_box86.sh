#!/bin/bash
cd /home/pi/box86
sudo cp /usr/local/bin/box86 box86/backup
cd box86/
git pull origin master
cd build
make -j4
sudo make install
exit


