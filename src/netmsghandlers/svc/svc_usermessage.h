#pragma once

#include "../netmsg_common.h"

class bf_read;
class leychan;

class svc_usermessage : public netmsg_common
{
public:
	svc_usermessage() : netmsg_common("svc_usermessage", 23) { }

	bool Register(leychan* chan);
	static bool ParseMessage(leychan* chan, svc_usermessage* thisptr, bf_read& msg);
};