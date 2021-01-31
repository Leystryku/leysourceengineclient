#pragma once

#include "../netmsg_common.h"

class bf_read;
class leychan;

class svc_print : public netmsg_common
{
public:
	svc_print() : netmsg_common("svc_print", 7) { }

	bool Register(leychan* chan);
	static bool ParseMessage(leychan* chan, svc_print* thisptr, bf_read& msg);
};