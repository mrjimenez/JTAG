
#include <PlayXSVFJTAGArduino.h>
//#include <PlayXSVFJTAGAVR.h>

//
// The maximum size of the arduino receive buffer is 63 bytes by default.
// This number can be changed if you change the compiler flags in the
// file hardware/arduino/avr/platform.local.txt (-O2 makes it faster):
//
// compiler.c.extra_flags=-DSERIAL_BUFFER_SIZE=256 -DSERIAL_RX_BUFFER_SIZE=256 -O2
// compiler.cpp.extra_flags=-DSERIAL_BUFFER_SIZE=256 -DSERIAL_RX_BUFFER_SIZE=256 -O2
//

#ifndef SERIAL_RX_BUFFER_SIZE
#define SERIAL_RX_BUFFER_SIZE 64
#endif /* SERIAL_RX_BUFFER_SIZE */

void setup()
{
	Serial.begin(115200);
	//Serial.begin(230400);
	while (!Serial) {
		; // wait for serial port to connect. Needed for Leonardo only
	}
	//printAvailableRAM(Serial);
}

void loop()
{
	//PlayXSVFJTAGArduino p(Serial, SERIAL_RX_BUFFER_SIZE, 2, 12, 5, 4, 0, true);
	PlayXSVFJTAGArduino p(Serial, SERIAL_RX_BUFFER_SIZE, 8, 9, 10, 11, 12);
	//PlayXSVFJTAGAVR p(Serial, SERIAL_RX_BUFFER_SIZE);
	//p.printAvailableRAM();
	// Currently, the PlayXSVFJTAGArduino object consumes 1042 bytes of RAM
	p.play();
}
