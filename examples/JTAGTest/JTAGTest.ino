
//#include <JTAGPortAVR.h>
#include <JTAGPortArduino.h>
#include <SerialComm.h>
#include <XSVFPlayerJTAG.h>

//
// The maximum size of the arduino receive buffer is 63 bytes by default.
// This number can be changed if you change the compiler flags in the
// file hardware/arduino/avr/platform.local.txt (-O2 makes it faster):
// 
// compiler.c.extra_flags=-DSERIAL_BUFFER_SIZE=256 -DSERIAL_RX_BUFFER_SIZE=256 -O2
// compiler.cpp.extra_flags=-DSERIAL_BUFFER_SIZE=256 -DSERIAL_RX_BUFFER_SIZE=256 -O2
//

void setup()
{
	Serial.begin(115200);
	while (!Serial) {
		; // wait for serial port to connect. Needed for Leonardo only
	}
}

void loop()
{
	SerialComm s(Serial, SERIAL_RX_BUFFER_SIZE);
	printFreeMemory(s);
	//JTAGPortAVR j;
	JTAGPortArduino j(8, 9, 10, 11, 12);
	XSVFPlayerJTAG p(s, j);
	
	uint32_t n = 0;
	while (true) {
		//p.test_code(2);
		++n;
		if (!p.handle_next_instruction()) {
			if (!p.reached_xcomplete()) {
				s.Important(F("*****************************"));
				s.Important(F("Failure at instruction #%5d"), n);
				s.Important(F("*****************************"));
			} else {
				s.Important(F("********"));
				s.Important(F("Success!"));
				s.Important(F("********"));
			}
			p.print_last_tdo();
			break;
		}
	}
	s.Important(F("Processed %d instructions."), n);
}

// variables created by the build process when compiling the sketch
extern int __bss_end;
extern void *__brkval;

void printFreeMemory(SerialComm &s)
{
	s.Important(F("Free memory: %d bytes."), memoryFree());
}

// function to return the amount of free RAM
int memoryFree()
{
	int freeValue;
	if ((int)__brkval == 0)
		freeValue = ((int)&freeValue) - ((int)&__bss_end);
	else
		freeValue = ((int)&freeValue) - ((int)__brkval);
	return freeValue;
}

