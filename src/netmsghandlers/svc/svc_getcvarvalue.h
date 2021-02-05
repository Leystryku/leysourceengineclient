#pragma once

#include "../netmsg_common.h"

class bf_read;
class leychan;

class svc_getcvarvalue : public netmsg_common
{
public:
	svc_getcvarvalue() : netmsg_common("svc_getcvarvalue", 31, sizeof(int)) { }

	bool Register(leychan* chan);
	static bool ParseMessage(leychan* chan, svc_getcvarvalue* thisptr, bf_read& msg);
};