#pragma once

#include "../netmsg_common.h"

class bf_read;
class leychan;

class net_disconnect : public netmsg_common
{
public:
	net_disconnect() : netmsg_common("net_disconnect", 1) { }

	bool Register(leychan* chan);
	static bool ParseMessage(leychan* chan, net_disconnect* thisptr, bf_read& msg);
};