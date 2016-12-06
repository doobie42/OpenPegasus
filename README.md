# OpenPegasus
Open Source Firmware for the Pegasus Touch 3D Printer.  At some point I will pick up a secondar BBB (maybe once the WiFi one is available) and update these instructions with everything I do.

   Developed on Linux kernal: 3.8.13-bone79

   DTO (as root) (YMMV):
      One time:
            dtc -O dtb -I dts -o /lib/firmware/openpegasus-00A0.dtbo  -b 0 -@ openpegasus-00A0.dtsi

      Each boot (this can be setup automatically to load, review BBB documentation):
            echo openpegasus > /sys/devices/bone_capemgr.9/slots 

   Compile:

        cd mainapp
        scons
        cd hardware
        ./compileLaser

   Calibrations (in progress) [calib.txt is a text file; you'll need one for all resins you use, at some point it will have include files so you can have shared config, but I need to figure out how to handle this with the firmware saving it, etc.

      Z axis:
         bin/gui calib.txt z
      Galvo (X/Y):
         bin/gui calib.txt galvo
 
   Running (from mainapp/)

      Non-GUI Run:

        bin/gui calib.txt <gcode file>

      GUI run (unstable):

        bin/gui


Generating GCode

   I used slic3r v1.2.9  (configuration file: slic3r/pegasus.ini)


