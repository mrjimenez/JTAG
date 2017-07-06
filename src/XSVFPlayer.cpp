
#include <XSVFPlayer.h>

#define ERR_MSG(x,y) case x: { ret = y; break; }
#define NAME_FOR(x) case x: return F(#x);
#define NAME_FOR_STATE(x) case STATE_##x: return F(#x);
#define DECODE(x) case x: if (decode_##x()) { return execute_##x(); };

XSVFPlayer::XSVFPlayer(SerialComm &s)
: m_serial_comm(s)
, m_next_state(0)
, m_sirsize_bits(0)
, m_sirsize_bytes(0)
, m_sdrsize_bits(0)
, m_sdrsize_bytes(0)
, m_length2_bits(0)
, m_length2_bytes(0)
, m_repeat(32)
, m_runtest(0)
, m_endir_state(STATE_RUN_TEST_IDLE)
, m_enddr_state(STATE_RUN_TEST_IDLE)
, m_xcomplete(false)
, m_instruction_counter(0)
, m_stream_sum(0)
, m_error_code(ERR_NO_ERROR)
{
	serialComm().ask_for_data();
}

XSVFPlayer::~XSVFPlayer()
{
	uint8_t checksum = (-streamSum()) & 0xFF;
	serialComm().Important(F("Checksum:  0x%02X/%lu."),
		checksum, serialComm().streamCount());
	serialComm().Important(F("Sum: 0x%08lX/%lu."),
		streamSum(), serialComm().streamCount());
	if (errorCode() == ERR_NO_ERROR && !xcomplete()) {
		setErrorCode(ERR_XCOMPLETE_NOT_REACHED);
	}
	serialComm().Quit(errorCode(), error_message(errorCode()));
}

#ifndef ARDUINO_ARCH_AVR
// All bytes must pass through this function
uint8_t XSVFPlayer::nextByte()
{
	int c = serialComm().nextByte();
	if (c != -1) {
		addStreamSum(c);
	} else {
		serialComm().Quit(ERR_SERIAL_PORT_TIMEOUT,
			F("Serial port timeout!"));
	}

	return static_cast<uint8_t>(c);
}
#endif // ARDUINO_ARCH_AVR

uint8_t XSVFPlayer::getNextByte()
{
	uint8_t i = nextByte();
	serialComm().Debug(F(".    BYTE:%12u - 0x%02X"), i, i);
	
	return i;
}

uint16_t XSVFPlayer::getNextWord()
{
	uint16_t i = 0;
	i  = ((uint16_t)nextByte()) << 8;
	i |= ((uint16_t)nextByte());
	serialComm().Debug(F(".    WORD:12%u - 0x%04X"), i, i);

	return i;
}

uint32_t XSVFPlayer::getNextLong()
{
	uint32_t i = 0;
	i  = ((uint32_t)nextByte()) << 24;
	i |= ((uint32_t)nextByte()) << 16;
	i |= ((uint32_t)nextByte()) << 8;
	i |= ((uint32_t)nextByte());
	serialComm().Debug(F(".   DWORD:%12lu - 0x%08lX"), i, i);

	return i;
}

void XSVFPlayer::getNextBytes(uint8_t *data, uint32_t count)
{
	serialComm().DebugStartMessage();
	serialComm().DebugContMessage(F(".     HEX:"));
	while (count--) {
		uint8_t c = nextByte();
		serialComm().DebugContMessage(F(" %02X"), c);
		*data++ = c;
	}
	serialComm().DebugContMessage(F("\n"));
}

const __FlashStringHelper *XSVFPlayer::error_message(int error_code)
{
	const __FlashStringHelper *ret = F("Unknown error message");
	switch (error_code) {
		ERR_MSG(ERR_NO_ERROR, F("No error"));
		ERR_MSG(ERR_SERIAL_PORT_TIMEOUT, F("Serial port timeout"));
		ERR_MSG(ERR_VREF_NOT_PRESENT, F("VRef not present"));
		ERR_MSG(ERR_XCOMPLETE_NOT_REACHED, F("XCOMPLETE not reached"));
		ERR_MSG(ERR_DR_CHECK_FAILED, F("DR check failed"));
	}

	return ret;
}

void XSVFPlayer::print_last_tdo() const
{
	serialComm().ImportantBits(F("!Last TDO:"),
		last_tdo(), last_dr_size_bits());
}

/*
 * Reads the next instruction from the serial port. Also reads any
 * remaining instruction parameters into the instruction buffer.
 */
bool XSVFPlayer::handle_next_instruction()
{
	uint8_t instruction = getNextByte();
	incrementInstructionCounter();
	setStringBuffer(instruction_name(instruction));
	serialComm().Debug(F("%d - Handling %s(0x%02X)"),
		instructionCounter(),
		stringBuffer(),
		instruction);
	switch (instruction) {
		DECODE(XCOMPLETE);
		DECODE(XTDOMASK);
		DECODE(XSIR);
		DECODE(XSDR);
		DECODE(XRUNTEST);
#if IMPLEMENT_XRESERVED
		DECODE(XRESERVED_5);
		DECODE(XRESERVED_6);
#endif // IMPLEMENT_XRESERVED
		DECODE(XREPEAT);
		DECODE(XSDRSIZE);
		DECODE(XSDRTDO);
#if IMPLEMENT_XSDRINC
		DECODE(XSETSDRMASKS);
		DECODE(XSDRINC);
#endif // IMPLEMENT_XSDRINC
		DECODE(XSDRB);
		DECODE(XSDRC);
		DECODE(XSDRE);
		DECODE(XSDRTDOB);
		DECODE(XSDRTDOC);
		DECODE(XSDRTDOE);
		DECODE(XSTATE);
		DECODE(XENDIR);
		DECODE(XENDDR);
		DECODE(XSIR2);
		DECODE(XCOMMENT);
		DECODE(XWAIT);
	default:
		serialComm().Important(F("Unimplemented instruction: %s(0x%02X)"),
			stringBuffer(), instruction);
		return false;
	}
}

#ifndef ARDUINO_ARCH_AVR
void XSVFPlayer::setStringBuffer(const __FlashStringHelper *s)
{
	PGM_P p = reinterpret_cast<PGM_P>(s);
	size_t n = strlen_P(p);
	if (n > S_STRING_BUFFER_SIZE - 1) {
		serialComm().Debug(
			F(">>>>>>>>>>>> String truncated by %d bytes."),
			n - S_STRING_BUFFER_SIZE + 1);
	}
	strncpy_P(m_string_buffer, p, S_STRING_BUFFER_SIZE);
	m_string_buffer[S_STRING_BUFFER_SIZE - 1] = 0;
}
#endif // ARDUINO_ARCH_AVR

const __FlashStringHelper *XSVFPlayer::instruction_name(uint8_t instruction)
{
	switch (instruction) {
		NAME_FOR(XCOMPLETE);
		NAME_FOR(XTDOMASK);
		NAME_FOR(XSIR);
		NAME_FOR(XSDR);
		NAME_FOR(XRUNTEST);
		NAME_FOR(XRESERVED_5);
		NAME_FOR(XRESERVED_6);
		NAME_FOR(XREPEAT);
		NAME_FOR(XSDRSIZE);
		NAME_FOR(XSDRTDO);
		NAME_FOR(XSETSDRMASKS);
		NAME_FOR(XSDRINC);
		NAME_FOR(XSDRB);
		NAME_FOR(XSDRC);
		NAME_FOR(XSDRE);
		NAME_FOR(XSDRTDOB);
		NAME_FOR(XSDRTDOC);
		NAME_FOR(XSDRTDOE);
		NAME_FOR(XSTATE);
		NAME_FOR(XENDIR);
		NAME_FOR(XENDDR);
		NAME_FOR(XSIR2);
		NAME_FOR(XCOMMENT);
		NAME_FOR(XWAIT);
	default:
		return F("Unknown instruction");
	}
}

const __FlashStringHelper *XSVFPlayer::state_name(uint8_t state)
{
	switch (state) {
		NAME_FOR_STATE(TEST_LOGIC_RESET);
		NAME_FOR_STATE(RUN_TEST_IDLE);
		NAME_FOR_STATE(SELECT_DR_SCAN);
		NAME_FOR_STATE(CAPTURE_DR);
		NAME_FOR_STATE(SHIFT_DR);
		NAME_FOR_STATE(EXIT1_DR);
		NAME_FOR_STATE(PAUSE_DR);
		NAME_FOR_STATE(EXIT2_DR);
		NAME_FOR_STATE(UPDATE_DR);
		NAME_FOR_STATE(SELECT_IR_SCAN);
		NAME_FOR_STATE(CAPTURE_IR);
		NAME_FOR_STATE(SHIFT_IR);
		NAME_FOR_STATE(EXIT1_IR);
		NAME_FOR_STATE(PAUSE_IR);
		NAME_FOR_STATE(EXIT2_IR);
		NAME_FOR_STATE(UPDATE_IR);
	default:
		return F("UNKNOWN_STATE");
	}
}

bool XSVFPlayer::decode_XCOMPLETE()
{
	setXcomplete(true);

	return true;
}

bool XSVFPlayer::decode_XTDOMASK()
{
	getNextBytes(tdoMask(), sdrsizeBytes());
	serialComm().DebugBytes(F("... TDO mask set to"),
		tdoMask(), sdrsizeBytes());

	return true;
}

bool XSVFPlayer::decode_XSIR()
{
	setSirsizeBits(getNextByte());
	if (sirsizeBytes() > S_MAX_CHAIN_SIZE_BYTES) {
		serialComm().Important(
			F("Requested IR size (%d bits) is greater than the maximum chain"
			" size supported by this programmer (%d bits)."),
			sirsizeBits(), S_MAX_CHAIN_SIZE_BITS);
		return false;
	}
	getNextBytes(tdi(), sirsizeBytes());

	return true;
}

bool XSVFPlayer::decode_XSDR()
{
	getNextBytes(tdi(), sdrsizeBytes());

	return true;
}

bool XSVFPlayer::decode_XRUNTEST()
{
	setRuntest(getNextLong());
	serialComm().Debug(F("... runtest set to %ld"), runtest());

	return true;
}

bool XSVFPlayer::decode_XRESERVED_5()
{
	return true;
}

bool XSVFPlayer::decode_XRESERVED_6()
{
	return true;
}

bool XSVFPlayer::decode_XREPEAT()
{
	setRepeat(getNextByte());
	serialComm().Debug(F("... repeat set to %d"), repeat());

	return true;
}

bool XSVFPlayer::decode_XSDRSIZE()
{
	setSdrSizeBits(getNextLong());
	if (sdrsizeBytes() > S_MAX_CHAIN_SIZE_BYTES) {
		serialComm().Important(
			F("Requested DR size (%lu bits) is greater than the maximum chain"
			" size supported by this programmer (%d bits)."),
			sdrsizeBits(), S_MAX_CHAIN_SIZE_BITS);
		return false;
	}
	serialComm().Debug(F("... sdrsize set to %lu bits (%lu bytes)"),
		sdrsizeBits(), sdrsizeBytes());

	return true;
}

bool XSVFPlayer::decode_XSDRTDO()
{
	getNextBytes(tdi(), sdrsizeBytes());
	getNextBytes(tdoExpected(), sdrsizeBytes());

	return true;
}

bool XSVFPlayer::decode_XSETSDRMASKS()
{
	getNextBytes(addressMask(), sdrsizeBytes());
	getNextBytes(dataMask(), sdrsizeBytes());
	
	return true;
}

#if IMPLEMENT_XSDRINC
//bool XSVFPlayer::decode_XSDRINC() { return ; }
#endif // IMPLEMENT_XSDRINC

bool XSVFPlayer::decode_XSDRB()
{
	getNextBytes(tdi(), sdrsizeBytes());

	return true;
}

bool XSVFPlayer::decode_XSDRC()
{
	getNextBytes(tdi(), sdrsizeBytes());

	return true;
}

bool XSVFPlayer::decode_XSDRE()
{
	getNextBytes(tdi(), sdrsizeBytes());

	return true;
}

bool XSVFPlayer::decode_XSDRTDOB()
{
	getNextBytes(tdi(), sdrsizeBytes());
	getNextBytes(tdoExpected(), sdrsizeBytes());

	return true;
}

bool XSVFPlayer::decode_XSDRTDOC()
{
	getNextBytes(tdi(), sdrsizeBytes());
	getNextBytes(tdoExpected(), sdrsizeBytes());

	return true;
}

bool XSVFPlayer::decode_XSDRTDOE()
{
	getNextBytes(tdi(), sdrsizeBytes());
	getNextBytes(tdoExpected(), sdrsizeBytes());

	return true;
}

bool XSVFPlayer::decode_XSTATE()
{
	setNextState(getNextByte());

	return true;
}

bool XSVFPlayer::decode_XENDIR()
{
	bool ret = true;
	uint8_t s = getNextByte();
	switch (s) {
	case 0:
		setEndirState(STATE_RUN_TEST_IDLE);
		break;
	case 1:
		setEndirState(STATE_PAUSE_IR);
		break;
	default:
		serialComm().Debug(F("... invalid XENDIR parameter: %d"), s);
		ret = false;
		break;
	}
	setStringBuffer(state_name(endirState()));
	serialComm().Debug(F("... endir state set to %s(%d)"),
		stringBuffer(), endirState());

	return ret;
}

bool XSVFPlayer::decode_XENDDR()
{
	bool ret = true;
	uint8_t s = getNextByte();
	switch (s) {
	case 0:
		setEnddrState(STATE_RUN_TEST_IDLE);
		break;
	case 1:
		setEnddrState(STATE_PAUSE_DR);
		break;
	default:
		serialComm().Debug(F("... invalid XENDDR parameter: %d"), s);
		ret = false;
		break;
	}
	setStringBuffer(state_name(enddrState()));
	serialComm().Debug(F("... enddr state set to %s(%d)"),
		stringBuffer(), enddrState());

	return ret;
}

bool XSVFPlayer::decode_XSIR2()
{
	setSirsizeBits(getNextWord());
	getNextBytes(tdi(), sirsizeBytes());

	return true;
}

bool XSVFPlayer::decode_XCOMMENT()
{
	serialComm().DebugStartMessage();
	serialComm().DebugContMessage(F("XCOMMENT:"));
	uint8_t c;
	while ((c = getNextByte())) {
		serialComm().DebugContMessage(F("%c"), c);
		execute_XCOMMENT_auxiliar(c);
	}
	serialComm().DebugContMessage(F("\n"));
	execute_XCOMMENT_auxiliar(c);

	return true;
}

bool XSVFPlayer::decode_XWAIT()
{
	setWaitStartState(getNextByte());
	setWaitEndState(getNextByte());
	setWaitTimeUsecs(getNextLong());

	return true;
}

