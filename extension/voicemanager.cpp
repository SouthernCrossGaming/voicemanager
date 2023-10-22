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
        delete[] pVoiceDataResult;
        return originalVoiceData;
    }

    int16_t encPos = STEAM_HEADER_SIZE + 2;

    for (DecodedChunk decoded : m_vecDecodedChunks)
    {
        EncodedChunk encoded = OpusEncode(decoded);
        if (encoded.data.size() <= 0)
        {
            delete[] pVoiceDataResult;
            return originalVoiceData;
        }

        auto encodedSize = encoded.data.size();
        char* pEncodedSize = (char*)&encodedSize;
        std::copy(pEncodedSize, pEncodedSize + sizeof(int16_t), &pVoiceDataResult[encPos]);
        encPos += 2;

        char* pIndex = (char*)&encoded.index;
        std::copy(pIndex, pIndex + sizeof(int16_t), &pVoiceDataResult[encPos]);
        encPos += 2;

        std::copy(encoded.data.data(), encoded.data.data() + encoded.data.size(), &pVoiceDataResult[encPos]);

        encPos += encoded.data.size();
    }

    int16_t voiceDataLength = encPos - STEAM_HEADER_SIZE - 2;
    if (voiceDataLength <= 0)
    {
        delete[] pVoiceDataResult;
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

    delete[] originalVoiceData;
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

    // What was this for, bot detection?
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
        case 6: // Opus
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
                int16_t encodedSize = *((int16_t*)&bytes[tpos]);
                encodedChunk.data = std::vector<uint8_t>(encodedSize);
                tpos += 2;

                if (encodedSize <= 0)
                {
                    smutils->LogError(myself, "Found a chunk with a size <= 0");
                    return;
                }

                encodedChunk.index = *((int16_t*)&bytes[tpos]);
                tpos += 2;

                for (int i = 0; i < encodedSize; i++)
                {
                    encodedChunk.data[i] = bytes[tpos + i];
                }

                DecodedChunk decodedChunk = this->OpusDecode(encodedChunk);
                if (decodedChunk.data.size() <= 0)
                {
                    return;
                }

                m_vecDecodedChunks.push_back(decodedChunk);
                tpos += encodedChunk.data.size();
            }
        }
        case 0: // Silence
        {
            return;
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
    int err = opus_decoder_ctl(m_Opus_Decoder, OPUS_SET_GAIN(m_gain));
    if (err < 0)
    {
        smutils->LogError(myself, "Something went wrong with initing opus decoder gain: %i", err);
        return false;
    }

    return true;
}

EncodedChunk VoiceManager::OpusEncode(DecodedChunk decoded)
{
    EncodedChunk encoded;
    encoded.index = decoded.index;
    uint8_t x[MAX_PACKET_SIZE];

    int compressedBytes = opus_encode(m_Opus_Encoder, decoded.data.data(), decoded.data.size(), x, (opus_int32)MAX_PACKET_SIZE);
    if (compressedBytes < 0)
    {
        smutils->LogError(myself, "Opus Encoder Failed with err: %s", ErrorToString(compressedBytes));
    }

    encoded.data = std::vector<uint8_t>(compressedBytes);
    for (int i = 0; i < compressedBytes; i++)
    {
        encoded.data[i] = x[i];
    }

    return encoded;
}

DecodedChunk VoiceManager::OpusDecode(EncodedChunk encoded)
{
    DecodedChunk decoded;
    decoded.index = encoded.index;
    int16_t x[MAX_PACKET_SIZE];

    int samples = opus_decode(m_Opus_Decoder, encoded.data.data(), encoded.data.size(), x, MAX_FRAMEBUFFER_SAMPLES, 0);
    if (samples < 0)
    {
        smutils->LogError(myself, "Opus Decoder Failed: %s", ErrorToString(samples));
    }

    decoded.data = std::vector<int16_t>(samples);
    for (int i = 0; i < samples; i++)
    {
        decoded.data[i] = x[i];
    }

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