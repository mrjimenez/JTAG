
#include <XSVFPlayer.h>

#define NAME_FOR(x) case x: return (F(#x));
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
{
	serialComm().ask_for_data();
}

/*
 * Reads the next instruction from the serial port. Also reads any
 * remaining instruction parameters into the instruction buffer.
 */
bool XSVFPlayer::handle_next_instruction()
{
	uint8_t instruction = serialComm().getNextByte();
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
		NAME_FOR(STATE_TEST_LOGIC_RESET);
		NAME_FOR(STATE_RUN_TEST_IDLE);
		NAME_FOR(STATE_SELECT_DR_SCAN);
		NAME_FOR(STATE_CAPTURE_DR);
		NAME_FOR(STATE_SHIFT_DR);
		NAME_FOR(STATE_EXIT1_DR);
		NAME_FOR(STATE_PAUSE_DR);
		NAME_FOR(STATE_EXIT2_DR);
		NAME_FOR(STATE_UPDATE_DR);
		NAME_FOR(STATE_SELECT_IR_SCAN);
		NAME_FOR(STATE_CAPTURE_IR);
		NAME_FOR(STATE_SHIFT_IR);
		NAME_FOR(STATE_EXIT1_IR);
		NAME_FOR(STATE_PAUSE_IR);
		NAME_FOR(STATE_EXIT2_IR);
		NAME_FOR(STATE_UPDATE_IR);
	default:
		return F("UNKNOWN_STATE");
	}
}

bool XSVFPlayer::decode_XCOMPLETE()
{
	setXcomplete(true);
	serialComm().Important(F("XCOMPLETE"));

	return true;
}

bool XSVFPlayer::decode_XTDOMASK()
{
	serialComm().getNextBytes(tdoMask(), sdrsizeBytes());
	serialComm().DebugBytes(F("... TDO mask set to"),
		tdoMask(), sdrsizeBytes());

	return true;
}

bool XSVFPlayer::decode_XSIR()
{
	setSirsizeBits(serialComm().getNextByte());
	if (sirsizeBytes() > S_MAX_CHAIN_SIZE_BYTES) {
		serialComm().Important(
			F("Requested IR size (%d bits) is greater than the maximum chain"
			" size supported by this programmer (%d bits)."),
			sirsizeBits(), S_MAX_CHAIN_SIZE_BITS);
		return false;
	}
	serialComm().getNextBytes(tdi(), sirsizeBytes());

	return true;
}

bool XSVFPlayer::decode_XSDR()
{
	serialComm().getNextBytes(tdi(), sdrsizeBytes());

	return true;
}

bool XSVFPlayer::decode_XRUNTEST()
{
	setRuntest(serialComm().getNextLong());
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
	setRepeat(serialComm().getNextByte());
	serialComm().Debug(F("... repeat set to %d"), repeat());

	return true;
}

bool XSVFPlayer::decode_XSDRSIZE()
{
	setSdrSizeBits(serialComm().getNextLong());
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
	serialComm().getNextBytes(tdi(), sdrsizeBytes());
	serialComm().getNextBytes(tdoExpected(), sdrsizeBytes());

	return true;
}

bool XSVFPlayer::decode_XSETSDRMASKS()
{
	serialComm().getNextBytes(addressMask(), sdrsizeBytes());
	serialComm().getNextBytes(dataMask(), sdrsizeBytes());
	
	return true;
}

#if IMPLEMENT_XSDRINC
//bool XSVFPlayer::decode_XSDRINC() { return ; }
#endif // IMPLEMENT_XSDRINC

bool XSVFPlayer::decode_XSDRB()
{
	serialComm().getNextBytes(tdi(), sdrsizeBytes());

	return true;
}

bool XSVFPlayer::decode_XSDRC()
{
	serialComm().getNextBytes(tdi(), sdrsizeBytes());

	return true;
}

bool XSVFPlayer::decode_XSDRE()
{
	serialComm().getNextBytes(tdi(), sdrsizeBytes());

	return true;
}

bool XSVFPlayer::decode_XSDRTDOB()
{
	serialComm().getNextBytes(tdi(), sdrsizeBytes());
	serialComm().getNextBytes(tdoExpected(), sdrsizeBytes());

	return true;
}

bool XSVFPlayer::decode_XSDRTDOC()
{
	serialComm().getNextBytes(tdi(), sdrsizeBytes());
	serialComm().getNextBytes(tdoExpected(), sdrsizeBytes());

	return true;
}

bool XSVFPlayer::decode_XSDRTDOE()
{
	serialComm().getNextBytes(tdi(), sdrsizeBytes());
	serialComm().getNextBytes(tdoExpected(), sdrsizeBytes());

	return true;
}

bool XSVFPlayer::decode_XSTATE()
{
	setNextState(serialComm().getNextByte());

	return true;
}

bool XSVFPlayer::decode_XENDIR()
{
	bool ret = true;
	uint8_t s = serialComm().getNextByte();
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
	uint8_t s = serialComm().getNextByte();
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
	setSirsizeBits(serialComm().getNextWord());
	serialComm().getNextBytes(tdi(), sirsizeBytes());

	return true;
}

bool XSVFPlayer::decode_XCOMMENT()
{
	serialComm().DebugStartMessage();
	serialComm().DebugContMessage(F("XCOMMENT:"));
	uint8_t c;
	while (c = serialComm().getNextByte()) {
		serialComm().DebugContMessage(F("%c"), c);
		execute_XCOMMENT_auxiliar(c);
	}
	serialComm().DebugContMessage(F("\n"));
	execute_XCOMMENT_auxiliar(c);

	return true;
}

bool XSVFPlayer::decode_XWAIT()
{
	setWaitStartState(serialComm().getNextByte());
	setWaitEndState(serialComm().getNextByte());
	setWaitTimeUsecs(serialComm().getNextLong());

	return true;
}

