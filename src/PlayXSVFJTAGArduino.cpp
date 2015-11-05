
#include <PlayXSVFJTAGArduino.h>

PlayXSVFJTAGArduino::PlayXSVFJTAGArduino(
	Stream &s,
	int stream_buffer_size,
	uint8_t tms,
	uint8_t tdi,
	uint8_t tdo,
	uint8_t tck,
	uint8_t vref,
	bool vref_is_analog)
: PlayXSVF()
, m_serial_comm(s, stream_buffer_size)
, m_jtag_port(tms, tdi, tdo, tck, vref, vref_is_analog)
, m_xsvf_player(serialComm(), jtagPort())
{
}

