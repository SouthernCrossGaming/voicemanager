#include "extension.h"

VoiceManagerClientState::VoiceManagerClientState()
{
    for (int level = 0; level < 4; level++)
    {
        opus_int32 gain;
        switch (level)
        {
        case 0:
            gain = LEVEL_LOWER;
            break;
        case 1:
            gain = LEVEL_LOW;
            break;
        case 2:
            gain = LEVEL_HIGH;
            break;
        case 3:
            gain = LEVEL_HIGHER;
            break;
        }

        m_manager[level] = VoiceManager(gain);
    }
};

VoiceManager* VoiceManagerClientState::GetVoiceManager(int level)
{
    return &m_manager[level];
}