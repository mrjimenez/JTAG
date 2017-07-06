
#ifndef JTAGWHISPERER_H
#define JTAGWHISPERER_H

#include <JTAGTAP.h>
#include <SerialComm.h>

#define IMPLEMENT_XSDRINC 0

class XSVFPlayer
{
private:
	SerialComm &m_serial_comm;

	uint8_t m_next_state;
	uint16_t m_sirsize_bits;
	uint16_t m_sirsize_bytes;
	uint32_t m_sdrsize_bits;
	uint32_t m_sdrsize_bytes;
	uint32_t m_length2_bits;
	uint32_t m_length2_bytes;
	uint8_t m_repeat;
	uint32_t m_runtest;
	uint8_t m_endir_state;
	uint8_t m_enddr_state;
	uint8_t m_wait_start_state;
	uint8_t m_wait_end_state;
	uint32_t m_wait_time_usecs;
	bool m_xcomplete;

	static const uint32_t S_MAX_CHAIN_SIZE_BYTES = 129;
	static const uint32_t S_MAX_CHAIN_SIZE_BITS = S_MAX_CHAIN_SIZE_BYTES * 8;
	uint8_t m_tdi[S_MAX_CHAIN_SIZE_BYTES];
	uint8_t m_tdo[S_MAX_CHAIN_SIZE_BYTES];
	uint8_t m_tdo_mask[S_MAX_CHAIN_SIZE_BYTES];
	uint8_t m_tdo_expected[S_MAX_CHAIN_SIZE_BYTES];
	uint8_t m_address_mask[S_MAX_CHAIN_SIZE_BYTES];
	uint8_t m_data_mask[S_MAX_CHAIN_SIZE_BYTES];

	uint16_t m_instruction_counter;
	uint32_t m_stream_sum;
	int m_error_code;

	// The following maximum buffer size has been tested to be an exact
	// value. If you change any program messages or strings, with debug
	// enabled you may get a ">>>>>>>>>>>> String truncated by XX bytes."
	// warning. Fix this buffer size accordingly.
	static const uint32_t S_STRING_BUFFER_SIZE = 23;
	char m_string_buffer[S_STRING_BUFFER_SIZE];

	enum e_XSVF_Instruction
	{
		XCOMPLETE = 0,
		XTDOMASK,
		XSIR,
		XSDR,
		XRUNTEST,
		XRESERVED_5,
		XRESERVED_6,
		XREPEAT,
		XSDRSIZE,
		XSDRTDO,
		XSETSDRMASKS,
		XSDRINC,
		XSDRB,
		XSDRC,
		XSDRE,
		XSDRTDOB,
		XSDRTDOC,
		XSDRTDOE,
		XSTATE,
		XENDIR,
		XENDDR,
		XSIR2,
		XCOMMENT,
		XWAIT
	};

        SerialComm &serialComm() { return m_serial_comm; }
	SerialComm &serialComm() const { return m_serial_comm; }

#ifdef ARDUINO_ARCH_AVR
	// All bytes must pass through this function
	uint8_t nextByte()
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
#else // ARDUINO_ARCH_AVR
	uint8_t nextByte();
#endif // ARDUINO_ARCH_AVR

	// Returns the next byte from the stream.
	uint8_t getNextByte();

	// Returns the next word from the stream.
	uint16_t getNextWord();

	// Returns the next double word from the stream.
	uint32_t getNextLong();
	
	// Stores the next count bytes from the stream into data.
	void getNextBytes(uint8_t *data, uint32_t count);

	const __FlashStringHelper *error_message(int error_code);

public:
	enum e_Error_Code
	{
		ERR_NO_ERROR = 0,
		//
		ERR_SERIAL_PORT_TIMEOUT = -1,
		//
		ERR_VREF_NOT_PRESENT = -10,
		//
		ERR_XCOMPLETE_NOT_REACHED = -100,
		ERR_DR_CHECK_FAILED = -101,
	};

	XSVFPlayer(SerialComm &s);
	
	~XSVFPlayer();

	bool reached_xcomplete() const { return xcomplete(); }

	uint32_t last_dr_size_bits() const { return sdrsizeBits(); }

	uint32_t last_dr_size_bytes() const { return sdrsizeBytes(); }

	const uint8_t *last_tdo() const { return m_tdo; }

	void print_last_tdo() const;

	bool handle_next_instruction();

	static uint32_t numBytes(uint32_t numBits)
	{
		return (numBits + 7) >> 3;
	}

protected:
	const char *stringBuffer() const { return m_string_buffer; }
#ifdef ARDUINO_ARCH_AVR
	void setStringBuffer(const __FlashStringHelper *s)
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
#else // ARDUINO_ARCH_AVR
	void setStringBuffer(const __FlashStringHelper *s);
#endif // ARDUINO_ARCH_AVR
	
	const __FlashStringHelper *instruction_name(uint8_t instruction);
	const __FlashStringHelper *state_name(uint8_t state);
	
	uint8_t nextState() const { return m_next_state; }
	void setNextState(uint8_t n) { m_next_state = n; }

	uint16_t sirsizeBits() const { return m_sirsize_bits; }
	void setSirsizeBits(uint16_t n)
	{
		m_sirsize_bits = n;
		m_sirsize_bytes = numBytes(n);
	}
	uint8_t sirsizeBytes() const { return m_sirsize_bytes; }

	uint32_t sdrsizeBits() const { return m_sdrsize_bits; }
	void setSdrSizeBits(uint32_t n)
	{
		m_sdrsize_bits = n;
		m_sdrsize_bytes = numBytes(n);
	}
	uint32_t sdrsizeBytes() const { return m_sdrsize_bytes; }

	uint32_t length2Bits() const { return m_length2_bits; }
	void setLength2Bits(uint32_t n) { m_length2_bits = n; }

	uint32_t length2Bytes() const { return m_length2_bytes; }

	uint8_t repeat() const { return m_repeat; }
	void setRepeat(uint8_t n) { m_repeat = n; }

	uint32_t runtest() const { return m_runtest; }
	void setRuntest(uint32_t n) { m_runtest = n; }

	uint8_t endirState() const { return m_endir_state; }
	void setEndirState(uint8_t n) { m_endir_state = n; }

	uint8_t enddrState() const { return m_enddr_state; }
	void setEnddrState(uint8_t n) { m_enddr_state = n; }

	uint8_t waitStartState() const { return m_wait_start_state; }
	void setWaitStartState(uint8_t n) { m_wait_start_state = n; }

	uint8_t waitEndState() const { return m_wait_end_state; }
	void setWaitEndState(uint8_t n) { m_wait_end_state = n; }

	uint32_t waitTimeUsecs() const { return m_wait_time_usecs; }
	void setWaitTimeUsecs(uint32_t n) { m_wait_time_usecs = n; }

	bool xcomplete() const { return m_xcomplete; }
	void setXcomplete(bool n) { m_xcomplete = n; }

	uint8_t *tdi() { return m_tdi; }
	uint8_t *tdo() { return m_tdo; }
	const uint8_t *tdo() const { return m_tdo; }
	uint8_t *tdoMask() { return m_tdo_mask; }
	uint8_t *tdoExpected() { return m_tdo_expected; }

	uint8_t *addressMask() { return m_address_mask; }
	uint8_t *dataMask() { return m_data_mask; }

	uint16_t instructionCounter() const { return m_instruction_counter; }
	void incrementInstructionCounter() { ++m_instruction_counter; }

	uint32_t streamSum() const { return m_stream_sum; }
	void addStreamSum(uint8_t n) { m_stream_sum += n; }

	int errorCode() const { return m_error_code; }
	void setErrorCode(int n) { m_error_code = n; }

	/*
	 * XSVF instruction decoders
	 */
	bool decode_XCOMPLETE();
	bool decode_XTDOMASK();
	bool decode_XSIR();
	bool decode_XSDR();
	bool decode_XRUNTEST();
	bool decode_XRESERVED_5();
	bool decode_XRESERVED_6();
	bool decode_XREPEAT();
	bool decode_XSDRSIZE();
	bool decode_XSDRTDO();
	bool decode_XSETSDRMASKS();
#if IMPLEMENT_XSDRINC
	bool decode_XSDRINC();
#endif // IMPLEMENT_XSDRINC
	bool decode_XSDRB();
	bool decode_XSDRC();
	bool decode_XSDRE();
	bool decode_XSDRTDOB();
	bool decode_XSDRTDOC();
	bool decode_XSDRTDOE();
	bool decode_XSTATE();
	bool decode_XENDIR();
	bool decode_XENDDR();
	bool decode_XSIR2();
	bool decode_XCOMMENT();
	bool decode_XWAIT();

	/*
	 * XSVF instruction executors
	 */
	virtual bool execute_XCOMPLETE() = 0;
	virtual bool execute_XTDOMASK() = 0;
	virtual bool execute_XSIR() = 0;
	virtual bool execute_XSDR() = 0;
	virtual bool execute_XRUNTEST() = 0;
	virtual bool execute_XRESERVED_5() = 0;
	virtual bool execute_XRESERVED_6() = 0;
	virtual bool execute_XREPEAT() = 0;
	virtual bool execute_XSDRSIZE() = 0;
	virtual bool execute_XSDRTDO() = 0;
	virtual bool execute_XSETSDRMASKS() = 0;
	virtual bool execute_XSDRINC() = 0;
	virtual bool execute_XSDRB() = 0;
	virtual bool execute_XSDRC() = 0;
	virtual bool execute_XSDRE() = 0;
	virtual bool execute_XSDRTDOB() = 0;
	virtual bool execute_XSDRTDOC() = 0;
	virtual bool execute_XSDRTDOE() = 0;
	virtual bool execute_XSTATE() = 0;
	virtual bool execute_XENDIR() = 0;
	virtual bool execute_XENDDR() = 0;
	virtual bool execute_XSIR2() = 0;
	virtual bool execute_XCOMMENT() = 0;
	virtual void execute_XCOMMENT_auxiliar(uint8_t c) = 0;
	virtual bool execute_XWAIT() = 0;
};

#endif  // JTAGWHISPERER_H

