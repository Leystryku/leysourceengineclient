#pragma once

#include "../netmsg_common.h"

class bf_read;
class leychan;

class net_setconvar : public netmsg_common
{
public:
	net_setconvar() : netmsg_common("net_setconvar", 5, sizeof(char) + sizeof(char) + sizeof(char)) { }

	bool Register(leychan* chan);
	static bool ParseMessage(leychan* chan, net_setconvar* thisptr, bf_read& msg);
};