#pragma once

#include "../netmsg_common.h"

class bf_read;
class leychan;

class svc_entitymessage : public netmsg_common
{
public:
	svc_entitymessage() : netmsg_common("svc_entitymessage", 24) { }

	bool Register(leychan* chan);
	static bool ParseMessage(leychan* chan, svc_entitymessage* thisptr, bf_read& msg);
};