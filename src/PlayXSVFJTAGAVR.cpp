
#include <PlayXSVFJTAGAVR.h>

PlayXSVFJTAGAVR::PlayXSVFJTAGAVR(
	Stream &s,
	int stream_buffer_size)
: PlayXSVF()
, m_serial_comm(s, stream_buffer_size)
, m_jtag_port()
, m_xsvf_player(serialComm(), jtagPort())
{
}

