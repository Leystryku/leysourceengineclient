#pragma once

#include "../netmsg_common.h"

class bf_read;
class leychan;

class net_nop : public netmsg_common
{
public:
	net_nop() : netmsg_common("net_nop", 0) { }

	bool Register(leychan* chan);
	static bool ParseMessage(leychan* chan, net_nop* thisptr, bf_read& msg);
};