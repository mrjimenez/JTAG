
#ifndef SERIALCOMM_H
#define SERIALCOMM_H

#include <Arduino.h>

#define DEBUG 0

class SerialComm
{
private:
	static const int S_FORMAT_BUFFER_SIZE = 128 + 1;

	Stream &m_serial;

        bool m_already_asked;

        char m_send_msg[6];
	char m_format_buffer[S_FORMAT_BUFFER_SIZE];

        unsigned int m_stream_buffer_capacity;
        unsigned int m_stream_buffer_threshold;
        unsigned int m_stream_buffer_checkpoint;
        uint32_t m_stream_count;

	Stream &serial() const { return m_serial; }

        bool alreadyAsked() const { return m_already_asked; }
        void setAlreadyAsked() { m_already_asked = true; }

	unsigned int streamBufferCapacity() const { return m_stream_buffer_capacity; }
	void setStreamBufferCapacity(unsigned int n) { m_stream_buffer_capacity = n; }
	
	unsigned int streamBufferThreshold() const { return m_stream_buffer_threshold; }
	void setStreamBufferThreshold(unsigned int n) { m_stream_buffer_threshold = n; }

	unsigned int streamBufferCheckpoint() const { return m_stream_buffer_checkpoint; }
	void updateCheckpoint()
	{
		m_stream_buffer_checkpoint += streamBufferThreshold();
		m_stream_buffer_checkpoint %= streamBufferCapacity();
	}

	void incStreamCount() { ++m_stream_count; }

	char *sendMsg() { return m_send_msg; }
	size_t sizeofSendMsg() const { return sizeof m_send_msg; }

public:
	SerialComm(Stream &s, int stream_buffer_size);
	~SerialComm();

	void ask_for_data();
	int nextByte();

	uint32_t streamCount() const { return m_stream_count; }

	void Important(const __FlashStringHelper *fmt, ...);
	void ImportantBits(const __FlashStringHelper *msg, const uint8_t *pb, uint32_t count_bits);
	void Quit(int error_code, const __FlashStringHelper *message) const;
	void print_bytes(const uint8_t *pb, uint32_t count, bool lf = false);

	void DebugStartMessage() const;
	void DebugContMessage(const __FlashStringHelper *ifsh, ...);
	void Debug(const __FlashStringHelper *fmt, ...);
	void DebugBytes(const __FlashStringHelper *s, const uint8_t* p, uint8_t n);

private:
	char *formatBuffer() { return m_format_buffer; }

	void checkAndRequestData(bool force = false);
	void Ready(const __FlashStringHelper *message) const;
};

#endif  // SERIALCOMM_H

