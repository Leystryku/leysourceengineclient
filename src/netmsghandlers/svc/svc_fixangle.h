#pragma once

#include "../netmsg_common.h"

class bf_read;
class leychan;

class svc_fixangle : public netmsg_common
{
public:
	svc_fixangle() : netmsg_common("svc_fixangle", 19, sizeof(short) + sizeof(short) + sizeof(short)) { }

	bool Register(leychan* chan);
	static bool ParseMessage(leychan* chan, svc_fixangle* thisptr, bf_read& msg);
};