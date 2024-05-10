
#ifndef PLAYXSVF_H
#define PLAYXSVF_H

#include <Arduino.h> // for Stream

class SerialComm;
class XSVFPlayer;

class PlayXSVF
{
protected:
	virtual SerialComm &serialComm() = 0;
	virtual XSVFPlayer &xsvfPlayer() = 0;

public:
	PlayXSVF() {}
	~PlayXSVF() {}

	int play();

	void printAvailableRAM();
};

void printAvailableRAM(Stream &s);
void printAvailableRAM(SerialComm &s);
int availableRAM();

#endif // PLAYXSVF_H
