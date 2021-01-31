#pragma once

#include "../netmsg_common.h"

class bf_read;
class leychan;

class svc_bspdecal : public netmsg_common
{
public:
	svc_bspdecal() : netmsg_common("svc_bspdecal", 21) { }

	bool Register(leychan* chan);
	static bool ParseMessage(leychan* chan, svc_bspdecal* thisptr, bf_read& msg);
};