#pragma once

#include "../netmsg_common.h"

class bf_read;
class leychan;

class svc_setpause : public netmsg_common
{
public:
	svc_setpause() : netmsg_common("svc_setpause", 11, 0) { }

	bool Register(leychan* chan);
	static bool ParseMessage(leychan* chan, svc_setpause* thisptr, bf_read& msg);
};