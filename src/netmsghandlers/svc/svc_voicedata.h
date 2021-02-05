#pragma once

#include "../netmsg_common.h"

class bf_read;
class leychan;

class svc_voicedata : public netmsg_common
{
public:
	svc_voicedata() : netmsg_common("svc_voicedata", 15, sizeof(char) + sizeof(char) + sizeof(short)) { }

	bool Register(leychan* chan);
	static bool ParseMessage(leychan* chan, svc_voicedata* thisptr, bf_read& msg);
	static void PlaybackAudio(char* voiceData, int lengthInBits);
};