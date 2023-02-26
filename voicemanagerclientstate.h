#include "smsdk_ext.h"
#include "voicemanager.h"
#include <iclient.h>

class VoiceManagerClientState
{
private:
	VoiceManager m_manager[4];

public:
	VoiceManagerClientState();
	VoiceManager* GetVoiceManager(int level);
};