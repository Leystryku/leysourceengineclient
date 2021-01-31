#pragma once

#include "../netmsg_common.h"

class bf_read;
class leychan;

class net_tick : public netmsg_common
{
public:
	net_tick() : netmsg_common("net_tick", 3) { }

	bool Register(leychan* chan);
	static bool ParseMessage(leychan* chan, net_tick* thisptr, bf_read& msg);
};