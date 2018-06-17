README for 'setblocksize' tool
------------------------------

General
-------
This is a tool for adjusting the blocksize of SCSI disks.
It is available under GPL. See 'LICENSE' file.

I have tested it with the following disks:
- Seagate Barracuda 4 (ST15150DC)
- Seagate Hawk 1LP (ST31200N)

Seagate told me, that all of their disks use the same scheme for changing the
blocksize (if they support it at all). It works like this:
1) Send a 'MODE SELECT' command with a block descriptor that contains the
   desired blocksize. The value on the Format page (03h) is not changeable! 
2) Do *nothing* else or the buffered new blocksize is canceled.
3) Send a 'FORMAT UNIT' command.
4) Now the Format page (03h) reflect the new setting.

I hope that this will work with disks from most other manufacturers too (Check
the manual of your disk to verify whether the blocksize can be changed and/or
which values are allowed). 


Requirements
------------
- SCSI disk ;-)
- Linux with SCSI generic support (sg driver)
  The sg2 or sg3 drivers as in Linux 2.2.6 or newer works
  (Kernels 2.2.10, 2.4.21 and 2.6.16.20 are tested)
- Kernel header files (in '/usr/src/linux/include')
- GNU make
- GCC


How to configure
----------------
The desired blocksize must be specified in 'setblocksize.c' (default is a
blocksize of 512 Bytes).
Note: Since V0.2 the new blocksize can be specified on the command line.

The SCSI command timeout is set to 2 hours. If your disk cannot complete a
'FORMAT UNIT' command within that time, you have to adjust the 'TIMEOUT'
constant in 'setblocksize.c'.


How to build
------------
Type 'make all' in this directory to build the binary.
Type 'make clean' to get back to source only state.


How to run
----------
The device file (normally '/dev/sg*') must be specified on the command line
when running the binary. Use 'sg_map' and/or 'sg_scan' from the sg_utils
package (http://www.torque.net/sg/index.html) to determine the correct device
file if you don't know it.
Check that you have adequate permission to nuke the data on the specified disk
and execute like this:
-------------------------------------------------------------------------------
./setblocksize /dev/sg4
-------------------------------------------------------------------------------
Note: Since V0.1 you get a last chance to abort before your data is lost.

Note: Since V0.2 the blocksize can be specified on the command line like this:
-------------------------------------------------------------------------------
./setblocksize -b2048 /dev/sg4
-------------------------------------------------------------------------------

Now the LED of the *disk* should be lit or flashing (My ST15150 flashes) ...
Note that the disk is not formatted by the host, but formats itself - therefore
the LED of the host adapter is off (there is no data transfer on the SCSI Bus).

IMPORTANT:
The LL-format can take some time ... be patient and do not manually interrupt.
If you do so the disk can become unuseable!

Use 'scsiinfo', 'scsi-config' or something like that to check the new blocksize
after 'setblocksize' has finished. Search for the Format page (03h).


2007-09-04  Michael Baeuerle <micha@hilfe-fuer-linux.de>
