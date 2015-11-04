#include <JTAGPortArduino.h>

JTAGPortArduino::JTAGPortArduino(
	uint8_t tms,
	uint8_t tdi,
	uint8_t tdo,
	uint8_t tck,
	uint8_t vref,
	bool vref_is_analog)
: m_tms(tms)
, m_tdi(tdi)
, m_tdo(tdo)
, m_tck(tck)
, m_vref(vref)
, m_vref_is_analog(vref_is_analog)
{
	JTAGPortArduino::tck().set();
}

