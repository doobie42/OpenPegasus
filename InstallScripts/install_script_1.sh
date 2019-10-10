#!/bin/sh -e

#Flash a 4.4.* bone kernel
#	https://debian.beagleboard.org/images/bone-debian-8.6-lxqt-4gb-armhf-2016-11-06-4gb.img.xz
#	This will have 4.4.30, I have also run with 4.4.39

git clone http://github.com/doobie42/OpenPegasus

#Configuring WiFi (if you have wireless):
#	sudo conmanctl
#	#connmanctl> tether wifi disable
#	#connmanctl> enable wifi
#	#connmanctl> scan wifi
#	#connmanctl> services
#	#connmanctl> agent on
#	#connmanctl> connect wifi_*_managed_psk
#	#connmanctl> quit

#Install "extra" tools required to compile.

sudo apt-get update
sudo apt-get install scons

#	Only needed for GUI:
#	sudo apt-get install gtkmm-3.0 (this wild cards a lot of stuff that might not be needed, TBD cleanup) 

sudo apt-get autoremove

#Setup BBB Device Tree:

#===> in arm am335x-boneblack.dts there is a */ that needs to be removed I think... lets try without removing it...

cd OpenPegasus/dtb-rebuilder/
sudo make
sudo make install

echo "At this point you need to know what beaglebone you have and add one of the dtb=<> lines into the file."
echo "Note: only black wireless and black non-wireless are supported"
echo "Wireless: dtb=am335x-boneblack-wireless.dtb"
echo "Non-Wireless: dtb=am335x-boneblack.dtb"
echo "Check to see if cape_universal=enable, if so, set it to disable"

#cd ..
#cd BBB_configuration
#sudo mv uEnv.txt /boot/uEnv.txt

# Comment out the cmdline= default
sudo bash -c "sed -i '/^cmdline/s/^/#/' /boot/uEnv.txt"

while true; do
    read -p "Are you using a Beaglebone Wireless (Y/N) >" yn
    case $yn in
        [Yy]* ) sudo bash -c 'echo "dtb=am335x-boneblack-wireless.dtb" >> /boot/uEnv.txt'; break;;
        [Nn]* ) sudo bash -c 'echo "dtb=am335x-boneblack.dtb" >> /boot/uEnv.txt'; break;;
        * ) echo "Please answer Y or N.";;
    esac
done

sudo bash -c 'echo "cmdline=coherent_pool=1M quiet cape_universal=false" >> /boot/uEnv.txt'
sudo bash -c 'echo "cape_disable=bone_capemgr.disable_partno=BB-BONELT-HDMI,BB-BONELT-HDMIN" >> /boot/uEnv.txt'
sudo bash -c 'echo "cape_enable=bone_capemgr.enable_partno=uio_pruss_enable:00A0" >> /boot/uEnv.txt'

while true; do
    read -p "Do you want to view or edit uEnv.txt (Y/N) >" yn
    case $yn in
        [Yy]* ) sudo nano /boot/uEnv.txt; break;;
        [Nn]* ) break;;
        * ) echo "Please answer Y or N.";;
    esac
done

# To check the file manually use:
# sudo nano /boot/uEnv.txt

# (can you see mine for a bbb wireless in BBB_configuration)
#		At this point you need to know what beaglebone you have and add one of the dtb=<> lines into the file.
#			Note: only black wireless and black non-wireless are supported
#			Wireless: dtb=am335x-boneblack-wireless.dtb
#			Non-Wireless: dtb=am335x-boneblack.dtb

# also add lines for prus:
#		cape_disable=bone_capemgr.disable_partno=BB-BONELT-HDMI,BB-BONELT-HDMIN
#		cape_enable=bone_capemgr.enable_partno=uio_pruss_enable:00A0

# Check to see if cape_universal=enable, if so, set it to disable
#		cmdline=coherent_pool=1M quiet cape_universal=false

#Update the INITRAMFS (any time you change a DTB in /boot):
cd /opt/scripts/
sudo git pull
cd tools/developers/
sudo ./update_initrd.sh

echo " Addding lines to:  /etc/modprobe.d/pruss-blacklist.conf "
sudo bash -c 'echo "blacklist pruss" >> /etc/modprobe.d/pruss-blacklist.conf'
sudo bash -c 'echo "blacklist pruss_intc" >> /etc/modprobe.d/pruss-blacklist.conf'
sudo bash -c 'echo "blacklist pru-rproc" >> /etc/modprobe.d/pruss-blacklist.conf'

while true; do
    read -p "Do you want to view or edit pruss-blacklist.conf (Y/N) >" yn
    case $yn in
        [Yy]* ) sudo nano /etc/modprobe.d/pruss-blacklist.conf; break;;
        [Nn]* ) break;;
        * ) echo "Please answer Y or N.";;
    esac
done

# To check the file manually use:
# sudo nano /etc/modprobe.d/pruss-blacklist.conf

#		blacklist pruss
#		blacklist pruss_intc
#		blacklist pru-rproc

while true; do
    read -p "You need to reboot, do it now? (Y/N) >" yn
    case $yn in
        [Yy]* ) sudo reboot; break;;
        [Nn]* ) exit;;
        * ) echo "Please answer Y or N.";;
    esac
done
