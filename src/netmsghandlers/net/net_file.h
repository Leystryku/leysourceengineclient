#pragma once

#include "../netmsg_common.h"

class bf_read;
class leychan;

class net_file : public netmsg_common
{
public:
	net_file() : netmsg_common("net_file", 2, sizeof(long) + sizeof(char)) { }

	bool Register(leychan* chan);
	static bool ParseMessage(leychan* chan, net_file* thisptr, bf_read& msg);
};