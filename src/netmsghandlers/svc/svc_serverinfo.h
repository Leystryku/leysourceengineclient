#pragma once

#include "../netmsg_common.h"

class bf_read;
class leychan;

class svc_serverinfo : public netmsg_common
{
public:
	svc_serverinfo() : netmsg_common("svc_serverinfo", 8) { }

	bool Register(leychan* chan);
	static bool ParseMessage(leychan* chan, svc_serverinfo* thisptr, bf_read& msg);
};