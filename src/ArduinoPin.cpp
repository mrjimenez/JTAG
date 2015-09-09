#include "ArduinoPin.h"

#include <Arduino.h>

ArduinoPin::ArduinoPin(uint8_t pin, uint8_t mode)
: m_bit(0)
, m_port(0)
, m_pin(pin)
, m_mode(mode)
{
	setBit(digitalPinToBitMask(pin));
	setPort(digitalPinToPort(pin));
	pinMode(ArduinoPin::pin(), ArduinoPin::mode());
}

ArduinoInputPin::ArduinoInputPin(uint8_t pin)
: ArduinoPin(pin, INPUT_PULLUP)
, m_in(portInputRegister(port()))
{
}

ArduinoOutputPin::ArduinoOutputPin(uint8_t pin)
: ArduinoPin(pin, OUTPUT)
, m_out(portOutputRegister(port()))
{
}

