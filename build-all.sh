#!/bin/sh

sudo apt install devscripts build-essential lintian dkms debhelper dh-dkms libjansson-dev linux-headers-generic gcc-multilib -y

cd ./haspd-aksparlnx-dkms
debuild -us -uc -tc -b
cd ..

cd ./haspd
debuild -us -uc -tc -b
cd ..

cd ./usb-vhci-hcd-dkms
debuild -us -uc -tc -b
cd ..

sudo dpkg --unpack ./usb-vhci*.deb

cd ./libusb-vhci
debuild -us -uc -tc -b
cd ..
sudo dpkg -i ./libusb-vhci0_0.8_amd64.deb
sudo dpkg -i ./libusb-vhci-dev_0.8_amd64.deb

cd ./usbhaspd
debuild -us -uc -tc -b
cd ..

cd ./usbhaspinfo
debuild -us -uc -tc -b
cd ..

rm -f *.build *.buildinfo *.changes *.ddeb *dbgsym*

sudo apt remove usb-vhci-hcd-dkms --purge -y
sudo apt remove libusb-vhci-dev --purge -y
sudo apt remove libusb-vhci0 --purge -y
