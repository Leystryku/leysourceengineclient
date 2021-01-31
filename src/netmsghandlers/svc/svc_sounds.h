#pragma once

#include "../netmsg_common.h"

class bf_read;
class leychan;

class svc_sounds : public netmsg_common
{
public:
	svc_sounds() : netmsg_common("svc_sounds", 17) { }

	bool Register(leychan* chan);
	static bool ParseMessage(leychan* chan, svc_sounds* thisptr, bf_read& msg);
};