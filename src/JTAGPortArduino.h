#ifndef JTAGPORTARDUINO_H
#define JTAGPORTARDUINO_H

#include <JTAGPort.h>

#include <Arduino.h>
#include <ArduinoPin.h>

/*
 * This is a slight slower version of JTAGPort, but does not require a
 * particular pin assignment.
 */
class JTAGPortArduino : public JTAGPort
{
private:
	ArduinoOutputPin m_tms;
	ArduinoOutputPin m_tdi;
	ArduinoInputPin m_tdo;
	ArduinoOutputPin m_tck;
	ArduinoInputPin m_vref;

	const ArduinoOutputPin &tms() const { return m_tms; }
	const ArduinoOutputPin &tdi() const { return m_tdi; }
	const ArduinoInputPin &tdo() const { return m_tdo; }
	const ArduinoOutputPin &tck() const { return m_tck; }
	const ArduinoInputPin &vref() const { return m_vref; }

public:
	JTAGPortArduino(uint8_t tms, uint8_t tdi, uint8_t tdo, uint8_t tck, uint8_t vref);
	~JTAGPortArduino() {}

	void pulse_clock()
	{
		tck().clr();
		delayMicroseconds(1);
		tck().set();
	}

	bool pulse_clock_and_read_tdo()
	{
		tck().clr();
		delayMicroseconds(1);
		uint8_t ret = tdo().read();
		tck().set();

		return ret;
	}

	void set_tms() { tms().set(); }
	void clr_tms() { tms().clr(); }
	void set_tdi() { tdi().set(); }
	void clr_tdi() { tdi().clr(); }
	bool read_vref() const { return vref().read(); }
};

#endif  // JTAGPORTARDUINO_H

