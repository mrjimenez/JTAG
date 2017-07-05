
#include <JTAGTAP.h>

/*
 * Low nibble : TMS == 0
 * High nibble: TMS == 1
 */

#define TMS_T(TMS_HIGH_STATE, TMS_LOW_STATE) (((TMS_HIGH_STATE) << 4) | (TMS_LOW_STATE))

static const uint8_t tms_transitions[] = {
	/* STATE_TEST_LOGIC_RESET */ TMS_T(STATE_TEST_LOGIC_RESET, STATE_RUN_TEST_IDLE),
	/* STATE_RUN_TEST_IDLE    */ TMS_T(STATE_SELECT_DR_SCAN,   STATE_RUN_TEST_IDLE),
	/* STATE_SELECT_DR_SCAN   */ TMS_T(STATE_SELECT_IR_SCAN,   STATE_CAPTURE_DR),
	/* STATE_CAPTURE_DR       */ TMS_T(STATE_EXIT1_DR,         STATE_SHIFT_DR),
	/* STATE_SHIFT_DR         */ TMS_T(STATE_EXIT1_DR,         STATE_SHIFT_DR),
	/* STATE_EXIT1_DR         */ TMS_T(STATE_UPDATE_DR,        STATE_PAUSE_DR),
	/* STATE_PAUSE_DR         */ TMS_T(STATE_EXIT2_DR,         STATE_PAUSE_DR),
	/* STATE_EXIT2_DR         */ TMS_T(STATE_UPDATE_DR,        STATE_SHIFT_DR),
	/* STATE_UPDATE_DR        */ TMS_T(STATE_SELECT_DR_SCAN,   STATE_RUN_TEST_IDLE),
	/* STATE_SELECT_IR_SCAN   */ TMS_T(STATE_TEST_LOGIC_RESET, STATE_CAPTURE_IR),
	/* STATE_CAPTURE_IR       */ TMS_T(STATE_EXIT1_IR,         STATE_SHIFT_IR),
	/* STATE_SHIFT_IR         */ TMS_T(STATE_EXIT1_IR,         STATE_SHIFT_IR),
	/* STATE_EXIT1_IR         */ TMS_T(STATE_UPDATE_IR,        STATE_PAUSE_IR),
	/* STATE_PAUSE_IR         */ TMS_T(STATE_EXIT2_IR,         STATE_PAUSE_IR),
	/* STATE_EXIT2_IR         */ TMS_T(STATE_UPDATE_IR,        STATE_SHIFT_IR),
	/* STATE_UPDATE_IR        */ TMS_T(STATE_SELECT_DR_SCAN,   STATE_RUN_TEST_IDLE),
};

#define BITSTR(A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P) ( \
	((uint16_t)(A) << 15) | \
	((uint16_t)(B) << 14) | \
	((uint16_t)(C) << 13) | \
	((uint16_t)(D) << 12) | \
	((uint16_t)(E) << 11) | \
	((uint16_t)(F) << 10) | \
	((uint16_t)(G) <<  9) | \
	((uint16_t)(H) <<  8) | \
	((uint16_t)(I) <<  7) | \
	((uint16_t)(J) <<  6) | \
	((uint16_t)(K) <<  5) | \
	((uint16_t)(L) <<  4) | \
	((uint16_t)(M) <<  3) | \
	((uint16_t)(N) <<  2) | \
	((uint16_t)(O) <<  1) | \
	((uint16_t)(P) <<  0) )

/*
 * The index of this vector is the current state. The i-th bit tells you the
 * value TMS must assume in order to go to state "i".

------------------------------------------------------------------------------------------------------------
|                        |   || F | E | D | C || B | A | 9 | 8 || 7 | 6 | 5 | 4 || 3 | 2 | 1 | 0 ||   HEX  |
------------------------------------------------------------------------------------------------------------
| STATE_TEST_LOGIC_RESET | 0 || 0 | 0 | 0 | 0 || 0 | 0 | 0 | 0 || 0 | 0 | 0 | 0 || 0 | 0 | 0 | 1 || 0x0001 |
| STATE_RUN_TEST_IDLE    | 1 || 1 | 1 | 1 | 1 || 1 | 1 | 1 | 1 || 1 | 1 | 1 | 1 || 1 | 1 | 0 | 1 || 0xFFFD |
| STATE_SELECT_DR_SCAN   | 2 || 1 | 1 | 1 | 1 || 1 | 1 | 1 | 0 || 0 | 0 | 0 | 0 || 0 | x | 1 | 1 || 0xFE03 |
| STATE_CAPTURE_DR       | 3 || 1 | 1 | 1 | 1 || 1 | 1 | 1 | 1 || 1 | 1 | 1 | 0 || x | 1 | 1 | 1 || 0xFFE7 |
| STATE_SHIFT_DR         | 4 || 1 | 1 | 1 | 1 || 1 | 1 | 1 | 1 || 1 | 1 | 1 | 0 || 1 | 1 | 1 | 1 || 0xFFEF |
| STATE_EXIT1_DR         | 5 || 1 | 1 | 1 | 1 || 1 | 1 | 1 | 1 || 0 | 0 | x | 0 || 1 | 1 | 1 | 1 || 0xFF0F |
| STATE_PAUSE_DR         | 6 || 1 | 1 | 1 | 1 || 1 | 1 | 1 | 1 || 1 | 0 | 1 | 1 || 1 | 1 | 1 | 1 || 0xFFBF |
| STATE_EXIT2_DR         | 7 || 1 | 1 | 1 | 1 || 1 | 1 | 1 | 1 || x | 0 | 0 | 0 || 1 | 1 | 1 | 1 || 0xFF0F |
| STATE_UPDATE_DR        | 8 || 1 | 1 | 1 | 1 || 1 | 1 | 1 | x || 1 | 1 | 1 | 1 || 1 | 1 | 0 | 1 || 0xFEFD |
| STATE_SELECT_IR_SCAN   | 9 || 0 | 0 | 0 | 0 || 0 | 0 | x | 1 || 1 | 1 | 1 | 1 || 1 | 1 | 1 | 1 || 0x01FF |
| STATE_CAPTURE_IR       | A || 1 | 1 | 1 | 1 || 0 | x | 1 | 1 || 1 | 1 | 1 | 1 || 1 | 1 | 1 | 1 || 0xF3FF |
| STATE_SHIFT_IR         | B || 1 | 1 | 1 | 1 || 0 | 1 | 1 | 1 || 1 | 1 | 1 | 1 || 1 | 1 | 1 | 1 || 0xF7FF |
| STATE_EXIT1_IR         | C || 1 | 0 | 0 | x || 0 | 1 | 1 | 1 || 1 | 1 | 1 | 1 || 1 | 1 | 1 | 1 || 0x87FF |
| STATE_PAUSE_IR         | D || 1 | 1 | 0 | 1 || 1 | 1 | 1 | 1 || 1 | 1 | 1 | 1 || 1 | 1 | 1 | 1 || 0xDFFF |
| STATE_EXIT2_IR         | E || 1 | x | 0 | 0 || 0 | 1 | 1 | 1 || 1 | 1 | 1 | 1 || 1 | 1 | 1 | 1 || 0x87FF |
| STATE_UPDATE_IR        | F || x | 1 | 1 | 1 || 1 | 1 | 1 | 1 || 1 | 1 | 1 | 1 || 1 | 1 | 0 | 1 || 0x7FFD |
------------------------------------------------------------------------------------------------------------

*/
static const uint16_t tms_map[] = {
/* STATE_TEST_LOGIC_RESET */ BITSTR(  0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 1  ),
/* STATE_RUN_TEST_IDLE    */ BITSTR(  1, 1, 1, 1,   1, 1, 1, 1,   1, 1, 1, 1,   1, 1, 0, 1  ),
/* STATE_SELECT_DR_SCAN   */ BITSTR(  1, 1, 1, 1,   1, 1, 1, 0,   0, 0, 0, 0,   0, 0, 1, 1  ),
/* STATE_CAPTURE_DR       */ BITSTR(  1, 1, 1, 1,   1, 1, 1, 1,   1, 1, 1, 0,   0, 1, 1, 1  ),
/* STATE_SHIFT_DR         */ BITSTR(  1, 1, 1, 1,   1, 1, 1, 1,   1, 1, 1, 0,   1, 1, 1, 1  ),
/* STATE_EXIT1_DR         */ BITSTR(  1, 1, 1, 1,   1, 1, 1, 1,   0, 0, 0, 0,   1, 1, 1, 1  ),
/* STATE_PAUSE_DR         */ BITSTR(  1, 1, 1, 1,   1, 1, 1, 1,   1, 0, 1, 1,   1, 1, 1, 1  ),
/* STATE_EXIT2_DR         */ BITSTR(  1, 1, 1, 1,   1, 1, 1, 1,   0, 0, 0, 0,   1, 1, 1, 1  ),
/* STATE_UPDATE_DR        */ BITSTR(  1, 1, 1, 1,   1, 1, 1, 0,   1, 1, 1, 1,   1, 1, 0, 1  ),
/* STATE_SELECT_IR_SCAN   */ BITSTR(  0, 0, 0, 0,   0, 0, 0, 1,   1, 1, 1, 1,   1, 1, 1, 1  ),
/* STATE_CAPTURE_IR       */ BITSTR(  1, 1, 1, 1,   0, 0, 1, 1,   1, 1, 1, 1,   1, 1, 1, 1  ),
/* STATE_SHIFT_IR         */ BITSTR(  1, 1, 1, 1,   0, 1, 1, 1,   1, 1, 1, 1,   1, 1, 1, 1  ),
/* STATE_EXIT1_IR         */ BITSTR(  1, 0, 0, 0,   0, 1, 1, 1,   1, 1, 1, 1,   1, 1, 1, 1  ),
/* STATE_PAUSE_IR         */ BITSTR(  1, 1, 0, 1,   1, 1, 1, 1,   1, 1, 1, 1,   1, 1, 1, 1  ),
/* STATE_EXIT2_IR         */ BITSTR(  1, 0, 0, 0,   0, 1, 1, 1,   1, 1, 1, 1,   1, 1, 1, 1  ),
/* STATE_UPDATE_IR        */ BITSTR(  0, 1, 1, 1,   1, 1, 1, 1,   1, 1, 1, 1,   1, 1, 0, 1  ),
};

JTAGTAP::JTAGTAP(SerialComm &s, JTAGPort &j)
: m_serial_comm(s)
, m_jtag_port(j)
, m_current_state(STATE_TEST_LOGIC_RESET)
{
	if (!jtagPort().read_vref()) {
		serialComm().Quit(-10,
			F("VREF is not present. Please, check the cable."));
	}
}

void JTAGTAP::shift_td(
	uint8_t *input_data,
	uint8_t *output_data,
	uint32_t data_bits,
	bool must_end)
{
	uint32_t bit_count = data_bits;
	uint32_t byte_count = numBytes(data_bits);
	serialComm().Debug(F("... shifting %lu bits (%lu bytes)"),
		data_bits, byte_count);
	serialComm().DebugBytes(F("... data:     "), input_data, byte_count);
	for (uint32_t i = 0; i < byte_count; ++i) {
		uint8_t byte_out = input_data[byte_count - 1 - i];
		uint8_t tdo_byte = 0;
		for (int j = 0; j < 8 && bit_count-- > 0; ++j) {
			if (bit_count == 0 && must_end) {
				jtagPort().set_tms();
				state_ack(1);
			}
			if (byte_out & 1) {
				jtagPort().set_tdi();
			} else {
				jtagPort().clr_tdi();
			}
			byte_out >>= 1;
			bool tdo = jtagPort().pulse_clock_and_read_tdo();
			tdo_byte |= tdo << j;
		}
		output_data[byte_count - 1 - i] = tdo_byte;
	}
}

void JTAGTAP::state_ack(bool tms)
{
	if (tms) {
		setCurrentState((tms_transitions[currentState()] >> 4) & 0xf);
	} else {
		setCurrentState(tms_transitions[currentState()] & 0xf);
	}
}

void JTAGTAP::state_step(bool tms)
{
	if (tms) {
		jtagPort().set_tms();
	} else {
		jtagPort().clr_tms();
	}
	jtagPort().pulse_clock();
	state_ack(tms);
}

void JTAGTAP::state_goto(int state)
{
	if (state == STATE_TEST_LOGIC_RESET) {
		for (int i = 0; i < 5; ++i) {
			state_step(true);
		}
	} else {
		while (currentState() != state) {
			state_step((tms_map[currentState()] >> state) & 1);
		}
	}
}

void JTAGTAP::wait_time(uint32_t microseconds)
{
	serialComm().Debug(F("... waiting %ld microseconds"), microseconds);
	uint32_t until = micros() + microseconds;
	while (microseconds--) {
		jtagPort().pulse_clock();
	}
	while (micros() < until) {
		jtagPort().pulse_clock();
	}
}

