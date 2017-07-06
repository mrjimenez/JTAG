#ifdef ARDUINO_ARCH_AVR

#ifndef JTAGPORTAVR_H
#define JTAGPORTAVR_H

#include <JTAGPort.h>

#include <Arduino.h>

/*
 * This is a faster version of the JTAGPort, but requires a fixed pin assignment.
 */
class JTAGPortAVR : public JTAGPort
{
private:
	enum
	{
		TMS  = _BV(PINB0),  // Arduino pin 8
		TDI  = _BV(PINB1),  // Arduino pin 9
		TDO  = _BV(PINB2),  // Arduino pin 10
		TCK  = _BV(PINB3),  // Arduino pin 11
		VREF = _BV(PINB4)   // Arduino pin 12
	};

	// The current PORTB state. We write this only when we twiddle TCK.
	uint8_t m_portb;

public:
	JTAGPortAVR()
	: m_portb(0)
	{
		DDRB = TMS | TDI | TCK;
	}

	~JTAGPortAVR() {}

	void pulse_clock()
	{
		clr_port(TCK);
		delayMicroseconds(1);
		set_port(TCK);
	}

	bool pulse_clock_and_read_tdo()
	{
		clr_port(TCK);
		delayMicroseconds(1);
		uint8_t pinb = PINB;
		set_port(TCK);

		return pinb & TDO;
	}

	void set_tms() { set_port(TMS); }
	void clr_tms() { clr_port(TMS); }
	void set_tdi() { set_port(TDI); }
	void clr_tdi() { clr_port(TDI); }
	bool read_vref() const { return PINB & VREF; }

private:
	void write_portb_if_tck(uint8_t pin) {
		if (pin == TCK) {
			PORTB = m_portb;
		}
	}

	void set_port(uint8_t pin) {
		m_portb |= pin;
		write_portb_if_tck(pin);
	}

	void clr_port(uint8_t pin) {
		m_portb &= ~pin;
		write_portb_if_tck(pin);
	}
};

#endif  // JTAGPORTAVR_H

#endif // ARDUINO_ARCH_AVR

