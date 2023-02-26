#include "voicemanager.h"

VoiceManager::VoiceManager()
{
    m_gain = 0;
    m_vecDecodedChunks = std::vector<DecodedChunk>();
}

VoiceManager::VoiceManager(opus_int32 gain)
{
    m_gain = gain;
    m_vecDecodedChunks = std::vector<DecodedChunk>();
}

uint8_t* VoiceManager::OnBroadcastVoiceData(IClient* pClient, int nBytes, const uint8_t* data, int* nBytesOut)
{
    *nBytesOut = 0;
    m_vecDecodedChunks.clear();

    uint8_t* originalVoiceData = new uint8_t[MAX_PACKET_SIZE]();
    std::copy(data, data + nBytes, &originalVoiceData[0]);

    uint8_t* pVoiceDataResult = new uint8_t[MAX_PACKET_SIZE]();
    std::copy(data, data + nBytes, &pVoiceDataResult[0]);

    ParseSteamVoicePacket(pVoiceDataResult, nBytes);

    if (m_vecDecodedChunks.size() <= 0 || !InitOpusEncoder(m_sampleRate))
    {
        return originalVoiceData;
    }

    int16_t encPos = STEAM_HEADER_SIZE + 2;

    for (DecodedChunk decoded : m_vecDecodedChunks)
    {
        EncodedChunk encoded = OpusCompress(decoded);
        if (encoded.size <= 0)
        {
            return originalVoiceData;
        }

        char* pEncodedSize = (char*)&encoded.size;
        std::copy(pEncodedSize, pEncodedSize + sizeof(int16_t), &pVoiceDataResult[encPos]);
        encPos += 2;

        char* pIndex = (char*)&encoded.index;
        std::copy(pIndex, pIndex + sizeof(int16_t), &pVoiceDataResult[encPos]);
        encPos += 2;

        std::copy(encoded.data, encoded.data + encoded.size, &pVoiceDataResult[encPos]);

        encPos += encoded.size;
    }

    int16_t voiceDataLength = encPos - STEAM_HEADER_SIZE - 2;
    if (voiceDataLength <= 0)
    {
        return originalVoiceData;
    }

    // Copy the voice data byte length to the output data, just after the header info
    char* pVoiceDataLength = (char*)&voiceDataLength;
    std::copy(pVoiceDataLength, pVoiceDataLength + sizeof(uint16_t), &pVoiceDataResult[STEAM_HEADER_SIZE]);

    uint32_t newCRC = CRC::Calculate(pVoiceDataResult, encPos, CRC::CRC_32());

    // Copy the new CRC to the end of the output data
    char* pNewCRC = (char*)&newCRC;
    std::copy(pNewCRC, pNewCRC + sizeof(uint32_t), &pVoiceDataResult[encPos]);

    *nBytesOut = STEAM_HEADER_SIZE + 2 + voiceDataLength + sizeof(uint32_t);
    return pVoiceDataResult;
}

void VoiceManager::ParseSteamVoicePacket(uint8_t* bytes, int numBytes)
{
    int numDecompressedSamples = 0;
    int pos = 0;
    if (numBytes < 4 + 4 + 4 + 1 + 2)
    {
        return;
    }

    int dataLen = numBytes - 4; // skip CRC

    uint32_t CRCdemo = *((uint32_t*)&bytes[dataLen]);
    uint32_t CRCdata = CRC::Calculate(bytes, dataLen, CRC::CRC_32());
    if (CRCdata != CRCdemo)
    {
        return;
    }

    pos += 4;
    uint32_t iSteamCommunity = *((uint32_t*)&bytes[pos]);
    pos += 4;

    if (iSteamCommunity != 0x1100001)
    {
        return;
    }

    while (pos < dataLen)
    {
        uint8_t payloadType = bytes[pos];
        pos++;

        switch (payloadType)
        {
        case 11: // Sample Rate
        {
            if (pos + 2 > dataLen)
            {
                return;
            }

            int16_t rate = *((int16_t*)&bytes[pos]);
            pos += 2;
            m_sampleRate = (opus_int32)rate;
            if (!this->InitOpusDecoder(rate))
            {
                return;
            }

            break;
        }
        case 6: // Opus??
        {
            if (pos + 2 > dataLen)
            {
                return;
            }

            int16_t dataSize = *((int16_t*)&bytes[pos]);
            pos += 2;

            int actualDataSize = numBytes - STEAM_HEADER_SIZE - CRC_SIZE - sizeof(dataSize);
            if (actualDataSize != dataSize)
            {
                smutils->LogError(myself, "Specified size of voice data (%i bytes) does not match actual size (%i bytes).", dataSize, actualDataSize);
                return;
            }

            int tpos = pos;
            int maxpos = tpos + dataSize;
            while (tpos <= (maxpos - 4))
            {
                EncodedChunk encodedChunk;
                encodedChunk.size = *((int16_t*)&bytes[tpos]);
                tpos += 2;

                if (encodedChunk.size <= 0)
                {
                    smutils->LogError(myself, "Found a chunk with a size <= 0");
                    return;
                }

                encodedChunk.index = *((int16_t*)&bytes[tpos]);
                tpos += 2;

                encodedChunk.data = &bytes[tpos];

                DecodedChunk decodedChunk = this->OpusDecompress(encodedChunk);
                if (decodedChunk.samples <= 0)
                {
                    return;
                }

                m_vecDecodedChunks.push_back(decodedChunk);
                tpos += encodedChunk.size;
            }
        }
        case 0: // Silence
        {
            // smutils->LogError(myself, "Encountered silence payload");
            return;
            // uint16_t numSamples = *((uint16_t *)&bytes[pos]);
            // int freeSamples = (sizeof(m_decodeBuffer) / sizeof(float)) - numDecompressedSamples;
            // numSamples = MIN(freeSamples, numSamples);
            // memset(&m_decodeBuffer[numDecompressedSamples], 0, numSamples * sizeof(float));
            // numDecompressedSamples += numSamples;
            // return numDecompressedSamples;
        }
        }
    }
}

bool VoiceManager::ConfigureEncoder()
{
    int err = opus_encoder_ctl(m_Opus_Encoder, OPUS_SET_BITRATE(BITRATE));
    if (err < 0)
    {
        smutils->LogError(myself, "Something went wrong with initing opus encoder ctl bitrate: %i", err);
        return false;
    }

    err = opus_encoder_ctl(m_Opus_Encoder, OPUS_SET_SIGNAL(OPUS_SIGNAL_VOICE));
    if (err < 0)
    {
        smutils->LogError(myself, "Something went wrong with initing opus encoder ctl signal: %i", err);
        return false;
    }

    // err = opus_encoder_ctl(m_Opus_Encoder, OPUS_SET_PREDICTION_DISABLED(1));
    // if (err < 0)
    // {
    //     smutils->LogError(myself, "Something went wrong with initing opus encoder ctl prediction disabled: %i", err);
    //     return false;
    // }

    // err = opus_encoder_ctl(m_Opus_Encoder, OPUS_SET_PHASE_INVERSION_DISABLED(1));
    // if (err < 0)
    // {
    //     smutils->LogError(myself, "Something went wrong with initing opus encoder ctl inverstion disabled: %i", err);
    //     return false;
    // }

    // err = opus_encoder_ctl(m_Opus_Encoder, OPUS_SET_INBAND_FEC(1));
    // if (err < 0)
    // {
    //     smutils->LogError(myself, "Something went wrong with initing opus encoder ctl fec: %i", err);
    //     return false;
    // }

    // err = opus_encoder_ctl(m_Opus_Encoder, OPUS_SET_PACKET_LOSS_PERC(50));
    // if (err < 0)
    // {
    //     smutils->LogError(myself, "Something went wrong with initing opus encoder ctl fec: %i", err);
    //     return false;
    // }

    err = opus_encoder_ctl(m_Opus_Encoder, OPUS_SET_BANDWIDTH(OPUS_BANDWIDTH_SUPERWIDEBAND));
    if (err < 0)
    {
        smutils->LogError(myself, "Something went wrong with initing opus encoder ctl bandwidth: %i", err);
        return false;
    }

    err = opus_encoder_ctl(m_Opus_Encoder, OPUS_SET_EXPERT_FRAME_DURATION(OPUS_FRAMESIZE_20_MS));
    if (err < 0)
    {
        smutils->LogError(myself, "Something went wrong with initing opus encoder ctl frame duration: %i", err);
        return false;
    }

    err = opus_encoder_ctl(m_Opus_Encoder, OPUS_SET_COMPLEXITY(10));
    if (err < 0)
    {
        smutils->LogError(myself, "Something went wrong with initing opus encoder ctl frame complexity: %i", err);
        return false;
    }

    return true;
}

bool VoiceManager::InitOpusEncoder(opus_int32 rate)
{
    if (!m_Opus_Encoder)
    {
        int err;
        m_Opus_Encoder = opus_encoder_create(rate, CHANNELS, APPLICATION, &err);
        err = opus_encoder_ctl(m_Opus_Encoder, OPUS_RESET_STATE);

        if (err < 0)
        {
            smutils->LogError(myself, "Something went wrong with initing opus encoder: %i", err);
            return false;
        }

        return ConfigureEncoder();
    }

    // Already initialized
    return true;
}

bool VoiceManager::InitOpusDecoder(opus_int32 rate)
{
    if (!m_Opus_Decoder)
    {
        int err;
        m_Opus_Decoder = opus_decoder_create(rate, CHANNELS, &err);

        if (err < 0)
        {
            smutils->LogError(myself, "Something went wrong with initing opus decoder: %i", err);
            return false;
        }

        return ConfigureDecoder();
    }

    // Already initialized
    return true;
}

bool VoiceManager::ConfigureDecoder()
{
    // int err = opus_decoder_ctl(m_Opus_Decoder, OPUS_SET_SIGNAL(OPUS_SIGNAL_MUSIC));
    // if (err < 0)
    // {
    //     smutils->LogError(myself, "Something went wrong with initing opus decoder signal: %i", err);
    //     return false;
    // }

    int err = opus_decoder_ctl(m_Opus_Decoder, OPUS_SET_GAIN(m_gain));
    if (err < 0)
    {
        smutils->LogError(myself, "Something went wrong with initing opus decoder gain: %i", err);
        return false;
    }

    return true;
}

EncodedChunk VoiceManager::OpusCompress(DecodedChunk decoded)
{
    EncodedChunk encoded;
    encoded.index = decoded.index;
    encoded.data = new uint8_t[MAX_PACKET_SIZE]();

    int compressedBytes = opus_encode(m_Opus_Encoder, decoded.data, decoded.samples, encoded.data, (opus_int32)MAX_PACKET_SIZE);
    if (compressedBytes < 0)
    {
        smutils->LogError(myself, "Opus Encoder Failed with err: %s", ErrorToString(compressedBytes));
    }

    encoded.size = compressedBytes;

    // if (CvarVoiceManagerEnableDebugLogs.GetBool())
    // {
    //     PrintPacketDetails("[NEW CHUNK]", encoded);
    // }

    return encoded;
}

DecodedChunk VoiceManager::OpusDecompress(EncodedChunk encoded)
{
    DecodedChunk decoded;
    decoded.index = encoded.index;
    decoded.data = new opus_int16[MAX_FRAMEBUFFER_SAMPLES]();

    // InitOpusDecoder(m_SampleRate);

    // if (CvarVoiceManagerEnableDebugLogs.GetBool())
    // {
    //     PrintPacketDetails("[CHUNK]", encoded);
    // }

    // int err = opus_packet_unpad(encoded.data, encoded.size);
    // if (err < 0)
    // {
    //     smutils->LogError(myself, "Failed to remove padding from packet: %s", ErrorToString(err));
    // }

    // why crash :<
    int samples = opus_decode(m_Opus_Decoder, encoded.data, encoded.size, decoded.data, MAX_FRAMEBUFFER_SAMPLES, 0);
    if (samples < 0)
    {
        smutils->LogError(myself, "Opus Decoder Failed: %s", ErrorToString(samples));
    }

    // opus_int32 g = 1000;
    // opus_val32 gain;
    // gain = celt_exp2(MULT16_16_P15(QCONST16(6.48814081e-4f, 25), g));
    // for (int i=0; i < MAX_FRAMEBUFFER_SAMPLES; i++)
    // {
    //     opus_val32 x;
    //     x = MULT16_32_P16(decoded.data[i],gain);
    //     decoded.data[i] = SATURATE(x, 32767);
    // }

    decoded.samples = samples;

    return decoded;
}

const char* VoiceManager::ErrorToString(int error)
{
    switch (error)
    {
    case OPUS_OK:
        return "OK";
    case OPUS_BAD_ARG:
        return "One or more invalid/out of range arguments.";
    case OPUS_BUFFER_TOO_SMALL:
        return "The mode struct passed is invalid.";
    case OPUS_INTERNAL_ERROR:
        return "An internal error was detected.";
    case OPUS_INVALID_PACKET:
        return "The compressed data passed is corrupted.";
    case OPUS_UNIMPLEMENTED:
        return "Invalid/unsupported request number.";
    case OPUS_INVALID_STATE:
        return "An encoder or decoder structure is invalid or already freed.";
    default:
        return "Unknown error code";
    }
}