#pragma once

#include "../netmsg_common.h"

class bf_read;
class leychan;

class svc_setview : public netmsg_common
{
public:
	svc_setview() : netmsg_common("svc_setview", 18, sizeof(char)) { }

	bool Register(leychan* chan);
	static bool ParseMessage(leychan* chan, svc_setview* thisptr, bf_read& msg);
};