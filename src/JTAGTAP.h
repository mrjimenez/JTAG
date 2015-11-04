
#ifndef JTAGTAP_H
#define JTAGTAP_H

#include <JTAGPort.h>
#include <SerialComm.h>

enum e_TAPState
{
	STATE_TEST_LOGIC_RESET =  0,
	STATE_RUN_TEST_IDLE,  //  1
	STATE_SELECT_DR_SCAN, //  2
	STATE_CAPTURE_DR,     //  3
	STATE_SHIFT_DR,       //  4
	STATE_EXIT1_DR,       //  5
	STATE_PAUSE_DR,       //  6
	STATE_EXIT2_DR,       //  7
	STATE_UPDATE_DR,      //  8
	STATE_SELECT_IR_SCAN, //  9 
	STATE_CAPTURE_IR,     // 10
	STATE_SHIFT_IR,       // 11
	STATE_EXIT1_IR,       // 12
	STATE_PAUSE_IR,       // 13
	STATE_EXIT2_IR,       // 14
	STATE_UPDATE_IR,      // 15
};

class JTAGTAP
{
private:
	SerialComm &m_serial_comm;
	JTAGPort &m_jtag_port;
	uint8_t m_current_state;

	SerialComm &serialComm() { return m_serial_comm; }

	JTAGPort &jtagPort() { return m_jtag_port; }

	uint8_t currentState() const { return m_current_state; }
	void setCurrentState(uint8_t n) { m_current_state = n; }

	static uint32_t numBytes(uint32_t numBits)
	{
		return (numBits + 7) >> 3;
	}

	void state_ack(bool tms);
	
	void state_step(bool tms);

public:
	JTAGTAP(SerialComm &s, JTAGPort &j);

	~JTAGTAP() {}

	void shift_td(
		uint8_t *input_data,
		uint8_t *output_data,
		uint32_t data_bits,
		bool must_end);
	void state_goto(int state);
	void wait_time(uint32_t microseconds);
};

#endif // JTAGTAP_H

