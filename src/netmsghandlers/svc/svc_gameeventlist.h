#pragma once

#include "../netmsg_common.h"

class bf_read;
class leychan;

class svc_gameeventlist : public netmsg_common
{
public:
	svc_gameeventlist() : netmsg_common("svc_gameeventlist", 30, sizeof(char)) { }

	bool Register(leychan* chan);
	static bool ParseMessage(leychan* chan, svc_gameeventlist* thisptr, bf_read& msg);
};