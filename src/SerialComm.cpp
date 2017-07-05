
#include <SerialComm.h>

#ifdef __AVR__
	#define PLATFORM_VSNPRINTF(...) vsnprintf_P(__VA_ARGS__)
#else
	#define PLATFORM_VSNPRINTF(...) vsnprintf(__VA_ARGS__)
#endif


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
}

void SerialComm::ask_for_data()
{
	if (!alreadyAsked()) {
		checkAndRequestData(true);
		setAlreadyAsked();
	}
}

void SerialComm::Important(const __FlashStringHelper *ifsh, ...)
{
	va_list args;
	va_start(args, ifsh);
	PLATFORM_VSNPRINTF(formatBuffer(), S_FORMAT_BUFFER_SIZE,
		(const char *)ifsh, args);
	va_end(args);

	serial().print(F("!"));
	serial().println(formatBuffer());
}

void SerialComm::ImportantBits(const __FlashStringHelper *msg, const uint8_t *pb,
	uint32_t count_bits)
{
	uint32_t count_bytes = (count_bits + 7) >> 3;
	serial().print(msg);
	print_bytes(pb, count_bytes);
	serial().print(F("/"));
	serial().print(count_bits);
	serial().println(F(" bits"));
}

void SerialComm::checkAndRequestData(bool force)
{
	if (force ||
	    streamCount() % streamBufferCapacity() == streamBufferCheckpoint()) {
		updateCheckpoint();
		int n = force ? streamBufferCapacity() : streamBufferThreshold();
		snprintf(sendMsg(), sizeofSendMsg(), "S%-4d", n);
		serial().print(F("\r\n"));
		serial().println(sendMsg());
		if (DEBUG) {
			// Print this so that an interrupted line is
			// still recognized by the python script.
			serial().print(F("D.   >>>   "));
		}
	}
}

int SerialComm::nextByte()
{
	// Wait for TIMEOUT milliseconds if the buffer is empty.
	//unsigned const int TIMEOUT = 1000; // milliseconds
	unsigned const int TIMEOUT = 3000; // milliseconds
	unsigned long end_time = millis() + TIMEOUT;
        while (serial().available() <= 0 && millis() < end_time) {
        }
        // Return an invalid instruction
	int c = -1;
	int n = serial().available();
	if (n > 0) {
		c = serial().read();
		incStreamCount();
		checkAndRequestData();
	} else {
		Important(F("count: %ld, threshold: %d, mod: %ld, avail: %d"),
			streamCount(), streamBufferThreshold(),
			streamCount() % streamBufferCapacity(),
			n);
	}

	return c;
}

// Not needed, left here as example.
#if 0
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
#endif

#if DEBUG == 1

void SerialComm::DebugStartMessage() const
{
	serial().print(F("D"));
}

void SerialComm::DebugContMessage(const __FlashStringHelper *ifsh, ...)
{
	va_list args;
	va_start(args, ifsh);
	PLATFORM_VSNPRINTF(formatBuffer(), S_FORMAT_BUFFER_SIZE,
		(const char *)ifsh, args);
	va_end(args);
	serial().print(formatBuffer());
}

void SerialComm::Debug(const __FlashStringHelper *ifsh, ...)
{
	va_list args;
	va_start(args, ifsh);
	PLATFORM_VSNPRINTF(formatBuffer(), S_FORMAT_BUFFER_SIZE,
		(const char *)ifsh, args);
	va_end(args);
	DebugStartMessage();
	serial().println(formatBuffer());
}

void SerialComm::DebugBytes(const __FlashStringHelper *s, const uint8_t *b,
	uint8_t n)
{
	DebugStartMessage();
	serial().print(s);
	print_bytes(b, n);
	serial().println();
}

#else

void SerialComm::DebugStartMessage() const {}
void SerialComm::DebugContMessage(const __FlashStringHelper * /*ifsh*/, ...) {}
void SerialComm::Debug(const __FlashStringHelper * /*fmt*/, ...) {}
void SerialComm::DebugBytes(const __FlashStringHelper * /*s*/, const uint8_t * /*b*/, uint8_t /*n*/) {}

#endif

void SerialComm::Ready(const __FlashStringHelper *message) const
{
	serial().print(F("\r\nR"));
	serial().println(message);
}

void SerialComm::Quit(int error_code, const __FlashStringHelper *message) const
{
	serial().print(F("\r\nQ"));
	serial().print(error_code);
	serial().print(F(","));
	serial().println(message);
}

void SerialComm::print_bytes(const uint8_t *pb, uint32_t count, bool lf)
{
	if (!count) {
		if (lf) {
			serial().println(F(" "));
		} else {
			serial().print(F(" "));
		}
		return;
	}
	const unsigned char *p = reinterpret_cast<const unsigned char *>(pb);
	const char *fmt_msg = " %02X";
	int fmt_msg_size = 3;
	while (count) {
		char *f = formatBuffer();
		int buf_size = S_FORMAT_BUFFER_SIZE;
		while (count && buf_size > 6) {
			snprintf(f, buf_size, fmt_msg, *p);
			p++;
			f += fmt_msg_size;
			buf_size -= fmt_msg_size;
			count--;
		}
		if (count && buf_size <= 6) {
			serial().print(formatBuffer());
		}
	}
	if (lf) {
		serial().println(formatBuffer());
	} else {
		serial().print(formatBuffer());
	}
}

