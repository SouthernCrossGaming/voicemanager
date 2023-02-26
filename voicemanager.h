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
	OpusDecoder* m_Opus_Decoder = NULL;
	OpusEncoder* m_Opus_Encoder = NULL;

public:
	VoiceManager();
	VoiceManager(opus_int32 gain);
	void DoTheThing();
	static void writeInt(std::ofstream& stream, const int& t)
	{
		stream.write((const char*)&t, sizeof(int));
	}
	static void writeShort(std::ofstream& stream, const short& t)
	{
		stream.write((const char*)&t, sizeof(short));
	}

	static void writeRawData(const char* outFile, uint8_t* buf, size_t bufSize)
	{
		std::ofstream stream(outFile, std::ios::binary);
		stream.write((const char*)buf, bufSize);
	}

	static void readRawData(const char* inFile, uint8_t* buf, size_t bufSize)
	{
		std::ifstream stream(inFile, std::ios::binary);
		stream.read((char*)buf, bufSize);
	}

	static void writeWAVData(const char* outFile, int16_t* buf, size_t bufSize,
		int sampleRate, short channels)
	{
		std::ofstream stream(outFile, std::ios::binary);
		stream.write("RIFF", 4);
		writeInt(stream, 36 + bufSize);
		stream.write("WAVE", 4);
		stream.write("fmt ", 4);
		writeInt(stream, 16);
		writeShort(stream, 1);		  // Format (1 = PCM)
		writeShort(stream, channels); // Channels //mono/sterio
		writeInt(stream, sampleRate);

		writeInt(stream, sampleRate * channels * sizeof(int16_t)); // Byterate
		writeShort(stream, channels * sizeof(int16_t));			   // Frame size
		writeShort(stream, 8 * sizeof(int16_t));				   // Bits per sample
		stream.write("data", 4);
		uint32_t sz = bufSize;
		stream.write((const char*)&sz, 4);
		stream.write((const char*)buf, bufSize);
	}

	static void PrintPacketDetails(const char* description, EncodedChunk encoded)
	{
		int bw = opus_packet_get_bandwidth(encoded.data);
		int channels = opus_packet_get_nb_channels(encoded.data);
		int numFrames = opus_packet_get_nb_frames(encoded.data, encoded.size);
		int samplesPerFrame = opus_packet_get_samples_per_frame(encoded.data, 24000);

		smutils->LogError(myself, "%s Frame Index: %i", description, encoded.index);
		smutils->LogError(myself, "%s Channels: %i", description, channels);
		smutils->LogError(myself, "%s Number of Frames: %i", description, numFrames);
		smutils->LogError(myself, "%s Samples per frame: %i", description, samplesPerFrame);
		smutils->LogError(myself, "%s Bandwidth: %i", description, bw);
	}

	static void PrintBytes(const char* description, int byteCount, uint8_t* arr)
	{
		int bytesPerRow = 32;
		int bytesPerColumn = 4;
		char row[128];
		snprintf(row, sizeof(row), "\n");

		smutils->LogError(myself, description);
		for (int i = 1; i <= byteCount; i++)
		{
			snprintf(row, sizeof(row), "%s %02X", row, (unsigned char)arr[i - 1]);

			if (i != 1 && i % bytesPerRow == 0)
			{
				smutils->LogError(myself, "%s", row);
				snprintf(row, sizeof(row), "");
			}
			else if (i != 1 && i % bytesPerColumn == 0)
			{
				snprintf(row, sizeof(row), "%s   ", row);
			}
		}
		smutils->LogError(myself, "%s", row);
		snprintf(row, sizeof(row), "\n");
	}

	uint8_t* OnBroadcastVoiceData(IClient* pClient, int nBytes, const uint8_t* data, int* nBytesOut);
	void ParseSteamVoicePacket(uint8_t* bytes, int numBytes);
	bool InitOpusDecoder(opus_int32 rate);
	bool ConfigureDecoder();
	bool InitOpusEncoder(opus_int32 rate);
	bool ConfigureEncoder();
	// bool InitSilkDecoder(int16_t rate);
	// bool InitSilkEncoder(int16_t rate);
	DecodedChunk OpusDecompress(EncodedChunk encodedData);
	EncodedChunk OpusCompress(DecodedChunk decodedChunk);
	const char* ErrorToString(int error);
	// int SilkDecompress(
	// 	const uint8_t* compressedData,
	// 	uint32_t compressedBytes,
	// 	int16_t* uncompressedData,
	// 	uint32_t maxUncompressedSamples);
	// void FugWithIt(int16_t* uncompressedData, const int samples);
	// void SilkCompress(
	// 	const int16_t* uncompressedData,
	// 	const int samplesIn,
	// 	uint8_t* compressedData,
	// 	short* bytesOut);
	// void SilkReset();
};