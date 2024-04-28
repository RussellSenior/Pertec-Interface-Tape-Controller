This is the repository for a controller to formatted Pertec tape interface.

The directories are as follows:

	doc - Documentation
	firmware - Controller firmware
	kicad - KiCAD schematics, board layout, etc.

The firmware can be built from source using the gcc-arm-none-eabi compiler
and the libopencm3 development library (libopencm3.org) which is covered by
the GNU General Public License v3.0, as is this project.

UPDATE APRIL 2024
-----------------

1. Some STM32F4VE MCU boards include a pullup resistor on USB pin D+; others
do not.  The latter do not work with the old code.  Changes have been made
to the usb driver to work with either type of board.

2. A "move" command has been added to facilitate renaming and moving files
and directories.
 
3. The "DUMP" command includes an optional "E" argument, which causes the
display to be EBCDIC rather than ASCII.
 
4. If USB operation is selected, the onboard SD card can be mounted on the
host system as a FAT32 device.  This facilitates file transfer.  Note that
this feature has been tested only on Linux. 
 
5. Observe that if the capacity of the SD card is more than 32GB, that the
"current directory" functions may not operate correctly.  This is a
limitation in the FATFS file manager implementation and may be updated in
the future.

