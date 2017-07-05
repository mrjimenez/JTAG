
#include <PlayXSVF.h>

#include <SerialComm.h>
#include <XSVFPlayer.h>

int PlayXSVF::play()
{
	SerialComm &s(serialComm());
	XSVFPlayer &p(xsvfPlayer());
	int ret = 0;
	uint32_t n = 0;
	while (true) {
		//p.test_code(2);
		++n;
		if (!p.handle_next_instruction()) {
			ret = p.reached_xcomplete();
			if (ret) {
				s.Important(F("********"));
				s.Important(F("Success!"));
				s.Important(F("********"));
			} else {
				s.Important(F("*****************************"));
				s.Important(F("Failure at instruction #%5d"), n);
				s.Important(F("*****************************"));
			}
			p.print_last_tdo();
			break;
		}
		yield();
	}
	s.Important(F("Processed %d instructions."), n);

	return ret;
}

void  PlayXSVF::printAvailableRAM()
{
	::printAvailableRAM(serialComm());
}

void printAvailableRAM(Stream &s)
{
	s.print(F("Available RAM: "));
	s.print(availableRAM());
	s.println(F(" bytes."));
}

void printAvailableRAM(SerialComm &s)
{
	s.Important(F("Available RAM: %d bytes."), availableRAM());
}

// variables created by the build process when compiling the sketch
extern int __bss_end;
extern void *__brkval;

// function to return the amount of free RAM
int availableRAM()
{
	int freeValue;
	if ((int)__brkval == 0)
		freeValue = ((int)&freeValue) - ((int)&__bss_end);
	else
		freeValue = ((int)&freeValue) - ((int)__brkval);
	return freeValue;
}

