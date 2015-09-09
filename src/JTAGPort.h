#ifndef JTAGPORT_H
#define JTAGPORT_H

/*
 * Making this class virtual makes the code bigger (18 bytes) and slower. But
 * it is the right thing to do if we want to reuse the code in a non-messy way.
 */

class JTAGPort
{
public:
	JTAGPort() {}
	virtual ~JTAGPort() {}

	virtual void pulse_clock() = 0;
	virtual bool pulse_clock_and_read_tdo() = 0;
	virtual void set_tms() = 0;
	virtual void clr_tms() = 0;
	virtual void set_tdi() = 0;
	virtual void clr_tdi() = 0;
	virtual bool read_vref() const = 0;
};

#endif  // JTAGPORT_H

