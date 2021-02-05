#pragma once

#include "../netmsg_common.h"

class bf_read;
class leychan;

class net_tick : public netmsg_common
{
public:
	net_tick() : netmsg_common("net_tick", 3, sizeof(long) + sizeof(short) + sizeof(short)) { }

	bool Register(leychan* chan);
	static bool ParseMessage(leychan* chan, net_tick* thisptr, bf_read& msg);
};