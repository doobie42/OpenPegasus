# OpenPegasus
Open Source Firmware for the Pegasus Touch 3D Printer

   Linux kernal: 3.8.13-bone79

   DTO (as root) (YMMV):
      One time:
            dtc -O dtb -I dts -o /lib/firmware/openpegasus-00A0.dtbo  -b 0 -@ openpegasus-00A0.dtsi
      Each boot:
      	   echo openpegasus > /sys/devices/bone_capemgr.9/slots 

   Compile:

        cd mainapp
        scons
	cd hardware
	./compileLaser

   Calibrations (in progress):

      bin/gui calib.txt z

   Running (from mainapp/)
      Non-GUI Run:

        bin/gui calib.txt <gcode file>

      GUI run (unstable):

        bin/gui


Generating GCode

   I used slic3r v.

   I use this configuration file: slic3r/pegasus.ini
