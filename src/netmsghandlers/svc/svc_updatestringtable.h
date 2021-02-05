#pragma once

#include "../netmsg_common.h"

class bf_read;
class leychan;

class svc_updatestringtable : public netmsg_common
{
public:
	svc_updatestringtable() : netmsg_common("svc_updatestringtable", 13, sizeof(short)) { }

	bool Register(leychan* chan);
	static bool ParseMessage(leychan* chan, svc_updatestringtable* thisptr, bf_read& msg);
};