#pragma once

#include "../netmsg_common.h"

class bf_read;
class leychan;

class svc_packetentities : public netmsg_common
{
public:
	svc_packetentities() : netmsg_common("svc_packetentities", 26) { }

	bool Register(leychan* chan);
	static bool ParseMessage(leychan* chan, svc_packetentities* thisptr, bf_read& msg);
};