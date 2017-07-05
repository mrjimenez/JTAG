
#include <XSVFPlayerJTAG.h>

XSVFPlayerJTAG::XSVFPlayerJTAG(SerialComm &s, JTAGPort &j)
: XSVFPlayer(s)
, m_serial_comm(s)
, m_jtag_tap(s, j)
, m_current_state(0)
{
}

bool XSVFPlayerJTAG::execute_XCOMPLETE()
{
	return false;
}

bool XSVFPlayerJTAG::execute_XTDOMASK()
{
	return true;
}

bool XSVFPlayerJTAG::execute_XSIR()
{
	sir();
	
	return true;
}

bool XSVFPlayerJTAG::execute_XSDR()
{
	return sdr(true, true, true, true);
}

bool XSVFPlayerJTAG::execute_XRUNTEST()
{
	return true;
}

bool XSVFPlayerJTAG::execute_XRESERVED_5()
{
	return true;
}

bool XSVFPlayerJTAG::execute_XRESERVED_6()
{
	return true;
}

bool XSVFPlayerJTAG::execute_XREPEAT()
{
	return true;
}

bool XSVFPlayerJTAG::execute_XSDRSIZE()
{
	return true;
}

bool XSVFPlayerJTAG::execute_XSDRTDO()
{
	return sdr(true, true, true, true);
}

bool XSVFPlayerJTAG::execute_XSETSDRMASKS()
{
	return true;
}

bool XSVFPlayerJTAG::execute_XSDRINC()
{
	return false;
}

bool XSVFPlayerJTAG::execute_XSDRB()
{
	return sdr(true, false, false, false);
}

bool XSVFPlayerJTAG::execute_XSDRC()
{
	return sdr(false, false, false, false);
}

bool XSVFPlayerJTAG::execute_XSDRE()
{
	return sdr(false, false, false, true);
}

bool XSVFPlayerJTAG::execute_XSDRTDOB()
{
	return sdr(true, true, false, false);
}

bool XSVFPlayerJTAG::execute_XSDRTDOC()
{
	return sdr(false, true, false, false);
}

bool XSVFPlayerJTAG::execute_XSDRTDOE()
{
	return sdr(false, true, false, true);
}

bool XSVFPlayerJTAG::execute_XSTATE()
{
	state_goto(nextState());

	return true;
}

bool XSVFPlayerJTAG::execute_XENDIR()
{
	return true;
}

bool XSVFPlayerJTAG::execute_XENDDR()
{
	return true;
}

bool XSVFPlayerJTAG::execute_XSIR2()
{
	sir();

	return true;
}

bool XSVFPlayerJTAG::execute_XCOMMENT()
{
	return true;
}

void XSVFPlayerJTAG::execute_XCOMMENT_auxiliar(uint8_t /*c*/)
{
}

bool XSVFPlayerJTAG::execute_XWAIT()
{
	state_goto(waitStartState());
	jtagTap().wait_time(waitTimeUsecs());
	state_goto(waitEndState());

	return true;
}

void XSVFPlayerJTAG::state_goto(int state)
{
	jtagTap().state_goto(state);
	setCurrentState(state);
	setStringBuffer(state_name(state));
	serialComm().Debug(F("... current state is now %s(%d)"),
		stringBuffer(), state);
}

void XSVFPlayerJTAG::sir()
{
	state_goto(STATE_SHIFT_IR);
	serialComm().Debug(F("... shifting into IR"));
	jtagTap().shift_td(tdi(), tdo(), sirsizeBits(), true);
	if (!runtest()) {
		state_goto(endirState());
	} else {
		state_goto(STATE_RUN_TEST_IDLE);
		jtagTap().wait_time(runtest());
	}
}

bool XSVFPlayerJTAG::sdr(bool must_begin, bool must_check, bool use_mask, bool must_end)
{
	int attempts_left = repeat();
	bool matched = false;

	if (must_begin) {
		state_goto(STATE_SHIFT_DR);
	}
	while (!matched && attempts_left-- >= 0) {
		serialComm().Debug(F("... shifting into DR"));
		jtagTap().shift_td(tdi(), tdo(), sdrsizeBits(), must_end);
		if (!must_check) {
			break;
		}
		matched = is_tdo_as_expected(use_mask);
		if (!matched) {
			// XAP058, page 14
			state_goto(STATE_PAUSE_DR);
			state_goto(STATE_SHIFT_DR);
			state_goto(STATE_RUN_TEST_IDLE);
			jtagTap().wait_time(runtest());
			//
			state_goto(STATE_SHIFT_DR);
			if (attempts_left >= 0) {
				serialComm().Debug(F("...... repeating: %d"),
					repeat() - attempts_left);
			}
		}
	}
	if (must_check && !matched) {
		setErrorCode(ERR_DR_CHECK_FAILED);
		serialComm().Important(F("DR check failed!"));
	}
	if (must_end && matched) {
		if (!runtest()) {
			state_goto(enddrState());
		} else {
			state_goto(STATE_RUN_TEST_IDLE);
			jtagTap().wait_time(runtest());
		}
	}

	return !must_check || (must_check && matched);
}

bool XSVFPlayerJTAG::is_tdo_as_expected(bool use_mask)
{
	serialComm().DebugBytes(F("... TDO mask: "),
		tdoMask(), sdrsizeBytes());
	serialComm().DebugBytes(F("... expecting:"),
		tdoExpected(), sdrsizeBytes());
	serialComm().DebugBytes(F("... received: "), tdo(), sdrsizeBytes());
	for (uint32_t i = 0; i < sdrsizeBytes(); ++i) {
		uint8_t expected = tdoExpected()[i];
		uint8_t actual = tdo()[i];
		if (use_mask) {
			expected &= tdoMask()[i];
			actual &= tdoMask()[i];
		}
		if (expected != actual) {
			serialComm().Debug(F("... NO MATCH!"));
			return false;
		}
	}
	serialComm().Debug(F("... match!"));

	return true;
}

void XSVFPlayerJTAG::test_code(int i)
{
	setSirsizeBits(8);
	tdi()[0] = 1;
	while (true) {
		switch (i) {
		case 1: {
			state_goto(STATE_TEST_LOGIC_RESET);
			*((uint32_t *)tdi()) = 0x01;
			//*((uint32_t *)tdi()) = 0x00;
			//*((uint32_t *)tdi()) = 0xBF;
			//*((uint32_t *)tdi()) = 0xFD;
			sir();
			//
			setRepeat(0);
			*((uint32_t *)tdoMask()) = 0xFF8FFF0F;
			setSdrSizeBits(0x20);
			*((uint32_t *)tdi()) = 0;
			*((uint32_t *)tdoExpected()) = 0x93F0E5F6;
			sdr(true, true, true, true);
			//
			uint32_t microseconds = 1000;
			uint32_t until = micros() + microseconds;
			while (micros() < until) {
			}
			break;
		}
		case 2:
			sdr(true, true, true, true);
			break;
		}
	}
}

