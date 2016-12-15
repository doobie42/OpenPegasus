# OpenPegasus
Open Source Firmware for the Pegasus Touch 3D Printer.  At some point I will pick up a secondar BBB (maybe once the WiFi one is available) and update these instructions with everything I do.  This is a mess at the moment, but I'll try to clean it up more in the near future.

Step one...before installing BBB in the Pegasus Touch... tape or hot glue the power jack (similiar to the originl BBB).  There is no voltage protection on the BBB and you'll likely fry something if you don't.
Step two: I recommend setting things up outside of the FSL first..
Step three: disk space may become limited, make sure you put a SD card in the BBB, and use it for storage (don't leave a BBB image SDcard in there unless you format it; it'll wipe you BBB on a reboot).
Step four: read these instructions over a few times, I haven't organized them well yet, so they are likely confusing.
Step five: feel free to ask me questions; I'll try to answer them ASAP.

   Developed on Linux kernel: 3.8.13-bone79 (will work to enable newest BBB in the near future)

   Setup on "fresh board":
      	 sudo apt-get update
   	 sudo apt-get install scons
	 sudo apt-get install gtkmm-3.0 (this wild cards a lot of stuff that might not be needed, TBD cleanup)
   	 sudo apt-get autoremove

 ** Note: I am working to upgrade to kernel 4.4.x, I have everything setup, but have not yet installled the BBB in my printer yet.
   
   DTO (as root) (YMMV):
      One time:
            dtc -O dtb -I dts -o /lib/firmware/openpegasus-00A0.dtbo  -b 0 -@ openpegasus-00A0.dtsi
      Each boot (this can be setup automatically to load, review BBB documentation):
            3.8.x: echo openpegasus > /sys/devices/bone_capemgr.9/slots
	    4.4.x: echo openpegasus > /sys/devices/platform/bone_capemgr/slots
      Enable VNC
	    x11vnc -auth /var/run/lightdm/root/\:0 >&/dev/null &

   Clone:
      git clone http://github.com/doobie42/OpenPegasus
   
   Compile:

        cd mainapp
        scons
        cd hardware
        ./compileLaser

   Calibrations (in progress) [calib.txt is a text file; you'll need one for all resins you use, at some point it will have include files so you can have shared config, but I need to figure out how to handle this with the firmware saving it, etc.

  *** IMPORTANT NOTE: be conservative with the 'belowlimit' value.  Set it to 0 and move down from there slowly.  I'll try to get a command line version of the calibration up.
 
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
	Command line galvo calibration:
	        bin/gui calib.txt galvo

      GUI run (unstable; doesn't have ability to load gcode file yet):

        bin/gui

Generating GCode

   I used slic3r v1.2.9  (configuration file: slic3r/pegasus.ini)


Setup.

	Flash with the following image (4.4.x kernel):
	      https://debian.beagleboard.org/images/bone-debian-8.6-lxqt-4gb-armhf-2016-11-06-4gb.img.xz


BBB image clean up:
I removed a number of packages and deleted some directories:
   bb-node-red-installer bone101 bonescript c9-core-installer doc-beaglebone-getting-started
   doc-beaglebonegreen-getting-started doc-seeed-bbgw-getting-started nodejs
   apache2-*
   (or don't install) libglibmm-2.4-doc
   (or don't install) libgtkmm-3.0-doc

   rm -rf ~root/.c9
   