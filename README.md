# OpenPegasus
Open Source Firmware for the Pegasus Touch 3D Printer.  At some point I will pick up a secondar BBB (maybe once the WiFi one is available) and update these instructions with everything I do.

   Developed on Linux kernal: 3.8.13-bone79 (will work to enable newest BBB in the near future)

   DTO (as root) (YMMV):
      One time:
            dtc -O dtb -I dts -o /lib/firmware/openpegasus-00A0.dtbo  -b 0 -@ openpegasus-00A0.dtsi

      Each boot (this can be setup automatically to load, review BBB documentation):
            echo openpegasus > /sys/devices/bone_capemgr.9/slots 
      Enable VNC
	    x11vnc -auth /var/run/lightdm/root/\:0 >&/dev/null &

   Compile:

        cd mainapp
        scons
        cd hardware
        ./compileLaser

   Calibrations (in progress) [calib.txt is a text file; you'll need one for all resins you use, at some point it will have include files so you can have shared config, but I need to figure out how to handle this with the firmware saving it, etc.

      Z axis:
         Best to be done in the GUI, but you'll need to manually copy results into calib.txt file
      Z axis validation (this is mainly for development, not calibration:
         bin/gui calib.txt z
      Galvo (X/Y) (gui does't work yet):
         bin/gui calib.txt galvo 
            q - quit
	    N - next point
	    P - previous point
            L - move laser left
            R - move laser right
            U - move laser up (back?)
            D - move laser down (front?)
            F - increase moving speed
            S - decrease moving speed
           ' '- (space) see laser
            a - show all dots
            W - write calibration file (note this will override your file and format for the calib.txt file)
     
   Running (from mainapp/)

      Non-GUI Run:

        Print a file:
	        bin/gui calib.txt <gcode file>
	Lift Z-axis ~2cm
	        bin/gui calib.txt zup

      GUI run (unstable; doesn't have ability to load gcode file yet):

        bin/gui

Generating GCode

   I used slic3r v1.2.9  (configuration file: slic3r/pegasus.ini)


Setup.

	Flash with the following image:
	      https://debian.beagleboard.org/images/bone-debian-8.6-lxqt-4gb-armhf-2016-11-06-4gb.img.xz
	Download: