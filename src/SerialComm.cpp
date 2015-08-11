
#include <SerialComm.h>

SerialComm::SerialComm(Stream &s, int stream_buffer_size)
: m_serial(s)
, m_already_asked(false)
, m_stream_buffer_capacity(stream_buffer_size - 1)
//                                                                // Benchmark:
//                                                                // send a file of size 22846 bytes.
//, m_stream_buffer_threshold(m_stream_buffer_capacity)           //  6.41 s
//, m_stream_buffer_threshold(m_stream_buffer_capacity * 15 / 16) //  6.21 s
//, m_stream_buffer_threshold(m_stream_buffer_capacity * 14 / 16) //  6.18 s
//, m_stream_buffer_threshold(m_stream_buffer_capacity * 13 / 16) //  6.13 s
//, m_stream_buffer_threshold(m_stream_buffer_capacity * 12 / 16) //  6.02 s
//, m_stream_buffer_threshold(m_stream_buffer_capacity * 11 / 16) //  6.02 s
, m_stream_buffer_threshold(m_stream_buffer_capacity * 10 / 16)   //  6.02 s
//, m_stream_buffer_threshold(m_stream_buffer_capacity *  9 / 16) //  6.02 s
//, m_stream_buffer_threshold(m_stream_buffer_capacity *  8 / 16) //  6.03 s
//, m_stream_buffer_threshold(m_stream_buffer_capacity *  7 / 16) //  6.03 s
//, m_stream_buffer_threshold(m_stream_buffer_capacity *  6 / 16) //  6.04 s
//, m_stream_buffer_threshold(m_stream_buffer_capacity *  5 / 16) //  6.06 s
//, m_stream_buffer_threshold(m_stream_buffer_capacity *  4 / 16) //  6.07 s
//, m_stream_buffer_threshold(m_stream_buffer_capacity *  3 / 16) //  6.11 s
//, m_stream_buffer_threshold(m_stream_buffer_capacity *  2 / 16) //  6.17 s
//, m_stream_buffer_threshold(m_stream_buffer_capacity *  1 / 16) //  6.37 s
//, m_stream_buffer_threshold(1)                                  // 22.81 s
, m_stream_buffer_checkpoint(0)
, m_stream_sum(0)
, m_stream_count(0)
{
	if (streamBufferThreshold() == 0) {
		setStreamBufferThreshold(streamBufferCapacity());
	}
	while (serial().available()) {
		serial().read();
	}
	Ready(F("XSVF"));
}

SerialComm::~SerialComm()
{
	uint8_t checksum = (-streamSum()) & 0xFF;
	Important(F("Checksum:  0x%02X/%lu."), checksum, streamCount());
	Important(F("Sum: 0x%08lX/%lu."), streamSum(), streamCount());
	Quit(F("Exiting!"));
}

void SerialComm::ask_for_data()
{
	if (!alreadyAsked()) {
		checkAndRequestData(true);
		setAlreadyAsked();
	}
}

uint8_t SerialComm::getNextByte()
{
	uint8_t i = nextByte();
	Debug(F(".    BYTE:%12u - 0x%02X"), i, i);
	
	return i;
}

uint16_t SerialComm::getNextWord()
{
	uint16_t i = 0;
	i  = ((uint16_t)nextByte()) << 8;
	i |= ((uint16_t)nextByte());
	Debug(F(".    WORD:12%u - 0x%04X"), i, i);

	return i;
}

uint32_t SerialComm::getNextLong()
{
	uint32_t i = 0;
	i  = ((uint32_t)nextByte()) << 24;
	i |= ((uint32_t)nextByte()) << 16;
	i |= ((uint32_t)nextByte()) << 8;
	i |= ((uint32_t)nextByte());
	Debug(F(".   DWORD:%12lu - 0x%08lX"), i, i);

	return i;
}

void SerialComm::getNextBytes(uint8_t *data, uint32_t count)
{
	DebugStartMessage();
	DebugContMessage(F(".     HEX:"));
	while (count--) {
		uint8_t c = nextByte();
		DebugContMessage(F(" %02X"), c);
		*data++ = c;
	}
	DebugContMessage(F("\n"));
}

void SerialComm::Important(const __FlashStringHelper *ifsh, ...)
{
	CopyToFlashBuffer(ifsh);

	va_list args;
	va_start(args, ifsh);
	const char *tmp = format(flashBuffer(), args);
	va_end(args);

	serial().print(F("!"));
	serial().println(tmp);
}

void SerialComm::checkAndRequestData(bool force)
{
	if (force ||
	    streamCount() % streamBufferCapacity() == streamBufferCheckpoint()) {
		updateCheckpoint();
		int n = force ? streamBufferCapacity() : streamBufferThreshold();
		snprintf(sendMsg(), sizeofSendMsg(), "S%-4d", n);
		//Ready(sendMsg());
		serial().print(F("\r\n"));
		serial().println(sendMsg());
		if (DEBUG) {
			// Print this so that an interrupted line is
			// still recognized by the python script.
			serial().print(F("D.   >>>   "));
		}
	}
}

uint8_t SerialComm::nextByte()
{
	// Wait for TIMEOUT milliseconds if the buffer is empty.
	//unsigned const int TIMEOUT = 1000; // milliseconds
	unsigned const int TIMEOUT = 3000; // milliseconds
	unsigned long end_time = millis() + TIMEOUT;
        while (serial().available() <= 0 && millis() < end_time) {
        }
        // Return an invalid instruction
	uint8_t c = -1;
	int n = serial().available();
	if (n > 0) {
		c = serial().read();
		addStreamSum(c);
		incStreamCount();
		checkAndRequestData();
	} else {
		Important(F("count: %ld, threshold: %d, mod: %ld, avail: %d"),
			streamCount(), streamBufferThreshold(),
			streamCount() % streamBufferCapacity(),
			n);
		Quit(F("Serial port timeout!"));
	}

	return c;
}

void SerialComm::CopyToFlashBuffer(const __FlashStringHelper *ifsh)
{
	PGM_P p = reinterpret_cast<PGM_P>(ifsh);
	// Truncate message to the flash buffer size
	strncpy_P(flashBuffer(), p, S_FLASH_BUFFER_SIZE);

	// Left here as example of use of pgm_read_byte().
	/*
	char *s = flashBuffer();
	char c;
	uint32_t n = S_FLASH_BUFFER_SIZE;
	while (n-- && (c = pgm_read_byte(p++))) {
		*s++ = c;
	}
	if (n) {
		*s = 0;
	} else {
		*--s = 0;
	}
	*/
}

const char *SerialComm::format(const char *fmt, va_list args)
{
	vsnprintf(m_format_buffer, S_FORMAT_BUFFER_SIZE, fmt, args);

	return m_format_buffer;
}

#if DEBUG == 1

void SerialComm::DebugStartMessage() const
{
	serial().print(F("D"));
}

void SerialComm::DebugContMessage(const __FlashStringHelper *ifsh, ...)
{
	CopyToFlashBuffer(ifsh);

	va_list args;
	va_start(args, ifsh);
	const char *tmp = format(flashBuffer(), args);
	va_end(args);

	serial().print(tmp);
}

void SerialComm::Debug(const __FlashStringHelper *ifsh, ...)
{
	CopyToFlashBuffer(ifsh);

	va_list args;
	va_start(args, ifsh);
	const char *tmp = format(flashBuffer(), args);
	va_end(args);

	DebugStartMessage();
	serial().println(tmp);
}

void SerialComm::DebugBytes(const __FlashStringHelper *s, const uint8_t *b, uint8_t n)
{
	DebugStartMessage();
	serial().print(s);
	print_bytes(b, n);
	serial().println();
}

#else

void SerialComm::DebugStartMessage() const {}
void SerialComm::DebugContMessage(const __FlashStringHelper *ifsh, ...) {}
void SerialComm::Debug(const __FlashStringHelper *fmt, ...) {}
void SerialComm::DebugBytes(const __FlashStringHelper *s, const uint8_t* p, uint8_t n) {}

#endif

void SerialComm::Ready(const char *message) const
{
	serial().print(F("\r\nR"));
	serial().println(message);
}

void SerialComm::Ready(const __FlashStringHelper *message) const
{
	serial().print(F("\r\nR"));
	serial().println(message);
}

void SerialComm::Quit(const __FlashStringHelper *message) const
{
	serial().print(F("\r\nQ"));
	serial().println(message);
}

void SerialComm::print_bytes(const uint8_t *pb, uint32_t count, bool lf)
{
	const unsigned char *p = reinterpret_cast<const unsigned char *>(pb);
	char *fmt_msg = " %02X";
	int fmt_msg_size = 3;
	while (count) {
		char *f = m_format_buffer;
		int buf_size = S_FORMAT_BUFFER_SIZE;
		while (count && buf_size > 6) {
			snprintf(f, buf_size, fmt_msg, *p);
			p++;
			f += fmt_msg_size;
			buf_size -= fmt_msg_size;
			count--;
		}
		if (count && buf_size <= 6) {
			serial().print(m_format_buffer);
		}
	}
	if (lf) {
		serial().println(m_format_buffer);
	} else {
		serial().print(m_format_buffer);
	}
}

