#!/bin/sh

sudo apt install devscripts build-essential lintian dkms debhelper dh-dkms libjansson-dev linux-headers-generic gcc-multilib -y

cd ./haspd-aksparlnx-dkms
debuild -us -uc -tc -b
cd ..

dpkg-deb --root-owner-group --build ./haspd haspd_8.53-ubuntu_amd64.deb

cd ./usb-vhci-hcd-dkms
debuild -us -uc -tc -b
cd ..

sudo dpkg --unpack ./usb-vhci*.deb

cd ./libusb-vhci
debuild -us -uc -tc -b
cd ..
rm -f *.build *.buildinfo *.changes *.ddeb  *dbgsym*
sudo dpkg -i ./libusb-vhci*.deb

cd ./usbhaspd
debuild -us -uc -tc -b
cd ..
rm -f *.build *.buildinfo *.changes *.ddeb

cd ./usbhaspinfo
debuild -us -uc -tc -b
cd ..
rm -f *.build *.buildinfo *.changes *.ddeb *dbgsym*

sudo apt remove usb-vhci-hcd-dkms --purge -y
sudo apt remove libusb-vhci --purge -y
