#ifndef PLAYXSVFJTAGARDUINO_H
#define PLAYXSVFJTAGARDUINO_H

#include <JTAGPortArduino.h>
#include <PlayXSVF.h>
#include <SerialComm.h>
#include <XSVFPlayerJTAG.h>

class PlayXSVFJTAGArduino : public PlayXSVF
{
private:
	SerialComm m_serial_comm;
	// This JTAG tap is a bit slower, but you can use any arduino pin.
	JTAGPortArduino m_jtag_port;
	XSVFPlayerJTAG m_xsvf_player;

	JTAGPort &jtagPort() { return m_jtag_port; }

protected:
	XSVFPlayer &xsvfPlayer() { return m_xsvf_player; }

public:
	PlayXSVFJTAGArduino(
		Stream &s,
		int stream_buffer_size,
		uint8_t tms,
		uint8_t tdi,
		uint8_t tdo,
		uint8_t tck,
		uint8_t vref,
		bool vref_is_analog = false);
	~PlayXSVFJTAGArduino() {}
	SerialComm &serialComm() { return m_serial_comm; }
};

#endif // PLAYXSVFJTAGARDUINO_H

