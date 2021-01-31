#pragma once

#include "../netmsg_common.h"

class bf_read;
class leychan;

class svc_tempentities : public netmsg_common
{
public:
	svc_tempentities() : netmsg_common("svc_tempentities", 27) { }

	bool Register(leychan* chan);
	static bool ParseMessage(leychan* chan, svc_tempentities* thisptr, bf_read& msg);
};