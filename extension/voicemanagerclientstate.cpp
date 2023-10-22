#include "extension.h"

VoiceManagerClientState::VoiceManagerClientState()
{
    for (int level = 0; level < 4; level++)
    {
        opus_int32 gain;
        switch (level)
        {
        case 0:
            gain = LEVEL_QUIETER;
            break;
        case 1:
            gain = LEVEL_QUIET;
            break;
        case 2:
            gain = LEVEL_LOUD;
            break;
        case 3:
            gain = LEVEL_LOUDER;
            break;
        }

        m_manager[level] = VoiceManager(gain);
    }
};

VoiceManager* VoiceManagerClientState::GetVoiceManager(int level)
{
    return &m_manager[level];
}