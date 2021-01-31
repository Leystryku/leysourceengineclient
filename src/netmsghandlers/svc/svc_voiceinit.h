#pragma once

#include "../netmsg_common.h"

class bf_read;
class leychan;

class svc_voiceinit : public netmsg_common
{
public:
	svc_voiceinit() : netmsg_common("svc_voiceinit", 14) { }

	bool Register(leychan* chan);
	static bool ParseMessage(leychan* chan, svc_voiceinit* thisptr, bf_read& msg);
};