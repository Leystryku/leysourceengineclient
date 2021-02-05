#pragma once

#include "../netmsg_common.h"

class bf_read;
class leychan;

class svc_createstringtable : public netmsg_common
{
public:
	svc_createstringtable() : netmsg_common("svc_createstringtable", 12, sizeof(char) + sizeof(short) + sizeof(char)) { }

	bool Register(leychan* chan);
	static bool ParseMessage(leychan* chan, svc_createstringtable* thisptr, bf_read& msg);
};