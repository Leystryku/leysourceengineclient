#pragma once

#include "../netmsg_common.h"

class bf_read;
class leychan;

class svc_prefetch : public netmsg_common
{
public:
	svc_prefetch() : netmsg_common("svc_prefetch", 28, 0) { }

	bool Register(leychan* chan);
	static bool ParseMessage(leychan* chan, svc_prefetch* thisptr, bf_read& msg);
};