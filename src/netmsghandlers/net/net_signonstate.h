#pragma once

#include "../netmsg_common.h"

class bf_read;
class leychan;

class net_signonstate : public netmsg_common
{
public:
	net_signonstate() : netmsg_common("net_signonstate", 6) { }

	bool Register(leychan* chan);
	static bool ParseMessage(leychan* chan, net_signonstate* thisptr, bf_read& msg);
};