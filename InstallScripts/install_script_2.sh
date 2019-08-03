#!/bin/sh -e

#Compile OpenPegasus:
cd OpenPegasus/mainapp
scons
cd hardware
./compileLaser
		
#		Note: there are 3 profiles I setup for the laser, they control the delay beween enabling/disable the laser and moving the galvo.  The slower will be more accurate, to change:
#		cd mainapp/hardware
#		rm profilegalvo.hp
#		ln -s profiles/<profile>.hp profilegalvo.hp
#		./compileLaser
#		   default.hp - slowest
#		   faster.hp  - medium
#		   fastest.hp - fastest (might move laser too quickly, but will print faster)
		   
#Run OpenPegasus (Right now there is no GUI, even though it is called GUI, I disabled it for the time being):
	
#	bin/gui <calib file> - would bring up GUI when available
#	bin/gui <calib file> z - debug tool for z axis
#	bin/gui <calib file> zup  - moves the Z axis up about 2 cm
#	bin/gui <calib file> galvo - galvo calibration (works OK, not perfect)
#	bin/gui <calib file> zcal - z axis calibration (not implemented)
#	bin/gui <calib file> <gcode file> - print gcode file
cd ..


#Trouble shooting:
#	prussdrv_open open failed
#	gui: prussdrv_open failed
#	     If you get this error, it means the PRU wasn't setup/enabled properly.

#	     Either modprobe uio_pruss (doesn't need reboot, but will need to be done every boot)
#	     Or add "uio_pruss" to /etc/modules (recommended; needs reboot)
	      

#	BBB- wireless
#	     Seems to hang or lock up
#	     	   Power management is likely on:
#		   1) Current boot: Execute: sudo /sbin/iwconfig wlan0 power off
#		   2) Every boot: Create a cron job with the following 
#		   	  @reboot sleep 60 && /sbin/iwconfig wlan0 power off