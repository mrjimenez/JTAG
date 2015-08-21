
#ifndef XSVFPLAYERJTAG_H
#define XSVFPLAYERJTAG_H

#include <JTAGTAP.h>
#include <SerialComm.h>
#include <XSVFPlayer.h>

class XSVFPlayerJTAG : public XSVFPlayer
{
private:
	SerialComm &m_serial_comm;
	JTAGTAP m_jtag_tap;
	uint8_t m_current_state;

        SerialComm &serialComm() { return m_serial_comm; }

	JTAGTAP &jtagTap() { return m_jtag_tap; }
	
	uint8_t currentState() const { return m_current_state; }
	void setCurrentState(uint8_t n) { m_current_state = n; }

	void state_goto(int state);
	void sir();
	bool sdr(bool must_begin, bool must_end, bool use_mask, bool must_check);
	bool is_tdo_as_expected(bool use_mask);

public:
	XSVFPlayerJTAG(SerialComm &s, JTAGPort &j);

	virtual ~XSVFPlayerJTAG() {}

	void test_code(int i);

protected:
	virtual bool execute_XCOMPLETE();
	virtual bool execute_XTDOMASK();
	virtual bool execute_XSIR();
	virtual bool execute_XSDR();
	virtual bool execute_XRUNTEST();
	virtual bool execute_XRESERVED_5();
	virtual bool execute_XRESERVED_6();
	virtual bool execute_XREPEAT();
	virtual bool execute_XSDRSIZE();
	virtual bool execute_XSDRTDO();
	virtual bool execute_XSETSDRMASKS();
	virtual bool execute_XSDRINC();
	virtual bool execute_XSDRB();
	virtual bool execute_XSDRC();
	virtual bool execute_XSDRE();
	virtual bool execute_XSDRTDOB();
	virtual bool execute_XSDRTDOC();
	virtual bool execute_XSDRTDOE();
	virtual bool execute_XSTATE();
	virtual bool execute_XENDIR();
	virtual bool execute_XENDDR();
	virtual bool execute_XSIR2();
	virtual bool execute_XCOMMENT();
	virtual void execute_XCOMMENT_auxiliar(uint8_t c);
	virtual bool execute_XWAIT();
};

#endif // XSVFPLAYERJTAG_H

