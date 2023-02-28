#include <iostream>
#include <fstream>
#include <opus/opus.h>
#include "CRC.h"
#include "smsdk_ext.h"
#include <iclient.h>
#include "defines.h"

struct EncodedChunk
{
	int16_t size;
	int16_t index;
	uint8_t* data;
};

struct DecodedChunk
{
	int16_t samples;
	int16_t index;
	opus_int16* data;
};

class VoiceManager
{

private:
	std::vector<DecodedChunk> m_vecDecodedChunks;
	opus_int32 m_sampleRate = 0;
	opus_int32 m_gain = 0;
	OpusDecoder* m_Opus_Decoder = nullptr;
	OpusEncoder* m_Opus_Encoder = nullptr;

public:
	VoiceManager();
	VoiceManager(opus_int32 gain);

	static void writeInt(std::ofstream& stream, const int& t)
	{
		stream.write((const char*)&t, sizeof(int));
	}

	static void writeShort(std::ofstream& stream, const short& t)
	{
		stream.write((const char*)&t, sizeof(short));
	}

	uint8_t* OnBroadcastVoiceData(IClient* pClient, int nBytes, const uint8_t* data, int* nBytesOut);
	void ParseSteamVoicePacket(uint8_t* bytes, int numBytes);
	bool InitOpusDecoder(opus_int32 rate);
	bool ConfigureDecoder();
	bool InitOpusEncoder(opus_int32 rate);
	bool ConfigureEncoder();
	DecodedChunk OpusDecode(EncodedChunk encodedData);
	EncodedChunk OpusEncode(DecodedChunk decodedChunk);
	const char* ErrorToString(int error);
};