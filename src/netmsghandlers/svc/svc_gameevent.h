#pragma once

#include "../netmsg_common.h"

class bf_read;
class leychan;

class svc_gameevent : public netmsg_common
{
public:
	svc_gameevent() : netmsg_common("svc_gameevent", 25) { }

	bool Register(leychan* chan);
	static bool ParseMessage(leychan* chan, svc_gameevent* thisptr, bf_read& msg);
};