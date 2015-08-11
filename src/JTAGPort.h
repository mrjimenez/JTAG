#ifndef JTAGPORT_H
#define JTAGPORT_H

/*
 * Dont make this class virtual. Reasons:
 * - It will consume 44 bytes of dynamic memory.
 * - It will make the code run about 6.3% slower.
 */
#ifdef BAD_IDEA
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
        virtual bool read_vref() = 0;
};
#endif // BAD_IDEA

#if defined(ARDUINO_ARCH_AVR)

    #include <JTAGPortAVR.h>

#elif defined(ARDUINO_ARCH_SAM)

    #include <JTAGPortSAM.h>

#else // Generic code
    class JTAGPort
    {
    public:
            JTAGPort() {}
            ~JTAGPort() {}

            void pulse_clock() {}
            bool pulse_clock_and_read_tdo() { return false; }
            void set_tms() {}
            void clr_tms() {}
            void set_tdi() {}
            void clr_tdi() {}
            bool read_vref() { return false; }
    };
#endif

#endif  // JTAGPORT_H

