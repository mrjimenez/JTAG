# The JTAG Library for Arduino

This library has four interesting things.

First, it is a JTAG test access point (TAP) manipulation library. The code
tries to be processor agnostic as much as possible, so that it can be used in
the future in other platforms. JTAG is a powerfull interface, there are many
things that you can do besides programming devices. For example, you can do
boundary scan tests, and even debug and take complete control of some hardware.

Second, it is a XSVF file player. That way, it is possible to program CPLDs and
FPGAs through the JTAG port.

Third, there is a XSVF disassembler written in python, so that you can learn
what is happening in the XSVF file you are using to program your device.

And finally, there is a XSVF assembler, also written in python, so that you can
recompile or write your own XSVF programs yourself in a maintainable and
documented way, since the grammar supports comments.

Remember that Arduino compatible devices are usually powered with 5 Volts,
while most CPLDs and FPGAs will use a 3.3 Volts interface. In my original
project, I have used 3 voltage dividers for TCK, TMS and TDI. Each divider
consisted of one 330 Ohms resistor in series with a 180 Ohms resistor. One leg
of the 330 Ohms resistor goes to the ground, one leg of the 180 Ohms resistor
goes to the Arduino. Where two legs meet, this is where you should connect your
JTAG cable.

Those seem to be a bit low values for a voltage divider, but consider the fact
that you will most likely have around 100 Ohms impedance in your JTAG
homemade cable, and higher resistor values will most likely create reflections
on your signals that will ruin the operation of the JTAG TAP.

TDO and VREF can be tied straight to the arduino pins, but I suggest using a
100 Ohms resistor on TDO, just to be sure to make reflections low. The big
problem here is not TDO itself, but the crosstalk that can happen between TDO
and TCK in your target board. Trust me, I've been there.

Using VREF is important so that your Arduino knows your JTAG cable it is actually
connected to something. Also, VREF could be used to power buffers to convert
the Arduino signals to the right voltage level whithout the resistors I
mentioned before.

The pin configuration I used was this:

| Signal | Arduino pin |
|:-------|:-----------:|
| TMS    |  8 |
| TDI    |  9 |
| TDO    | 10 |
| TCK    | 11 |
| VREF   | 12 |


Before using this software
==========================

You will need:

1. An Arduino compatible board, along with some way to program it (PC with USB
   and Arduino IDE or some other hardware device).

1. Python with the library pyserial installed.

1. The proper connection, be it a cable or a few loose wire jumpers.

1. A CPLD, FPGA or any other JTAG interface board to connect to. It is worth
   mentioning that [Dangerous Prototypes](http://dangerousprototypes.com/)
   has two very interesting CPLD boards. I have personally used a XC2C64A
   breakout board. They also have a similar breakout board for the XC9572XL.


Installing the library
======================

This software has been tested under the Arduino IDE version 1.6.5, but should
work fine with version 1.0. The library should be unpacked to the "libraries"
directory inside your Arduino "sketches" directory.


Compiling
=========

One quick suggestion: in order to get the most of the USB bandwidth, it is
possible to increase the size of the receive buffer of the Arduino. I have also
changed the default compiler optimization level. I added a file called
"platform.local.txt" to the folder "~/arduino-1.6.5/hardware/arduino/avr" that
had the following:
```
name=Arduino 256S AVR Boards
version=1.6.7

# AVR compile variables
# ---------------------

# This can be overriden in boards.txt
build.extra_flags=

# These can be overridden in platform.local.txt
compiler.c.extra_flags=-DSERIAL_BUFFER_SIZE=256 -DSERIAL_RX_BUFFER_SIZE=256 -O2
compiler.c.elf.extra_flags=
compiler.S.extra_flags=
compiler.cpp.extra_flags=-DSERIAL_BUFFER_SIZE=256 -DSERIAL_RX_BUFFER_SIZE=256 -O2
compiler.ar.extra_flags=
compiler.objcopy.eep.extra_flags=
compiler.elf2hex.extra_flags=
```

The best information about the location of the file "platform.local.txt" I
found was [here](https://github.com/arduino/Arduino/wiki/Boards-Manager-FAQ).

Another suggestion: if speed is really an issue, instead of using
"JTAGPortArduino", use "JTAGPortAVR". Take a look at PlayXSVFJTAVR.{cpp,h}.
JTAGPortAVR uses the same pin configuration I have mentioned before, but since
all the pins are on AVR's PORTB, all pins are written at once, so the JTAG port
code is more efficient.

Using it to play XSVF
=====================

1. The example sketch "JTAGTest" has been used in all my tests. Upload it to
   your Arduino board. Now you have a XSVF JTAG player.

1. Inside the folder libraries/JTAG/extras/python, there is a python script
   called xsvf. It has some command line parameters, but the default ones
   should be ok. You can use it to send more than one file in a sequence to
   your board. For example, suppose you are in a command prompt in the
   libraries/JTAG folder. Then, the following command will send the following
   four XSVF files to your Arduino board:
```
   $ ./extras/python/xsvf extras/xsvf/XC2C64A/{idcode.xsvf,blank_check.xsvf,erase.xsvf,VHDL-CPLDIntro3LEDinverse.xsvf}
```
   Help is available for the command line parameters:
```
	$ ./extras/python/xsvf --help
	usage: xsvf [-h] [-c {upload,disasm}] [-v] [-p PORT] [-b BAUD]
            	    fileName [fileName ...]

	XSVF file processor.

	positional arguments:
  	  fileName              XSVF file names. (type FileType('rb'))

	optional arguments:
  	  -h, --help            show this help message and exit
  	  -c {upload,disasm}, --command {upload,disasm}
                        	command to execute. (default=upload)
  	  -v, --version         show program's version number and exit
  	  -p PORT, --port PORT  Serial port device name (default=/dev/ttyACM0)
  	  -b BAUD, --baud BAUD  BAUD rate (type int, default=115200)

	Parameters can be in a file, one per line, using @"file name"
```
1. In subfolders of the folder "extras/xsvf" you can find several XSVF files
   that you can use to test your hardware. Notice that you MUST use a XSVF file
   generated for your particular hardware. You can generate XSVF files with the
   software "impact", which comes with the Xilinx bundle.

1. A failure in the execution of "blank_check.xsvf" means that the CPLD is not
   blank. The same file should be successful after the execution of
   "erase.xsvf".


Interesting links
=================

1. [Generating an SVF, XSVF, or STAPL File with Impact](http://www.xilinx.com/support/documentation/sw_manuals/xilinx11/pp_p_process_generate_svf_file.htm)

1. [Executing XSVF/SVF Files with Impact](http://www.xilinx.com/itp/xilinx10/isehelp/pim_p_executing_xsvf_svf.htm)

1. [A JTAG/XSVF Library for Arduino](http://eeandcs.blogspot.com.br/2015/08/jtagxsvf-library-for-arduino.html)

1. [A XSVF Assembler/Disassembler in python](http://eeandcs.blogspot.com.br/2015/09/a-xsvf-assemblerdisassembler-in-python.html)

1. [SVF on Wikipedia](https://en.wikipedia.org/wiki/Serial_Vector_Format)

1. [JTAG on Wikipedia](https://en.wikipedia.org/wiki/Joint_Test_Action_Group)

1. [fpga4fun](http://www.fpga4fun.com/JTAG.html)

1. [Rohit Dureja's JTAG Simplified](https://rohitdureja.wordpress.com/category/cpldfpga/)

1. [Nice JTAG TAP Controller State Diagram in SVF](https://de.wikipedia.org/wiki/Datei:JTAG_TAP_Controller_State_Diagram.svg)

1. Youtube video: [Como programar CPLD Xilinx con Arduino](https://youtu.be/x5eW9H1GoeA)


Credits
=======

I have used Xilinx [XAPP503 - SVF and XSVF File Formats for Xilinx Devices]
(http://www.xilinx.com/support/documentation/application_notes/xapp503.pdf),
appendix B as the reference for XSVF.

This software is actually a major rewrite of code and ideas inspired on many
other projects. As far as I could trace it:

1. [Mike Tsao's JTAGWhisperer](https://github.com/sowbug/JTAGWhisperer/).

1. [Ben's XSVF-Player](https://github.com/ben0109/XSVF-Player/).

1. [Clifford Wolf's libxsvf](http://www.clifford.at/libxsvf/). Definitely worth taking a look.

1. [Karl Hans Janke's jtagbang](http://www.khjk.org/log/2013/aug/jtagbang.html).


About me
========

[My blog is here](http://eeandcs.blogspot.com.br/).

You may contact me on github.

