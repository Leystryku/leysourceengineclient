#pragma once

#include "../netmsg_common.h"

class bf_read;
class leychan;

class svc_classinfo : public netmsg_common
{
public:
	svc_classinfo() : netmsg_common("svc_classinfo", 10) { }

	bool Register(leychan* chan);
	static bool ParseMessage(leychan* chan, svc_classinfo* thisptr, bf_read& msg);
};