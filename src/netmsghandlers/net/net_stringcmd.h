#pragma once

#include "../netmsg_common.h"

class bf_read;
class leychan;

class net_stringcmd : public netmsg_common
{
public:
	net_stringcmd() : netmsg_common("net_stringcmd", 4, sizeof(char)) { }

	bool Register(leychan* chan);
	static bool ParseMessage(leychan* chan, net_stringcmd* thisptr, bf_read& msg);
};