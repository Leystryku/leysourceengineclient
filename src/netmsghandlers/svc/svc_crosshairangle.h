#pragma once

#include "../netmsg_common.h"

class bf_read;
class leychan;

class svc_crosshairangle : public netmsg_common
{
public:
	svc_crosshairangle() : netmsg_common("svc_crosshairangle", 20, sizeof(short) + sizeof(short) + sizeof(short)) { }

	bool Register(leychan* chan);
	static bool ParseMessage(leychan* chan, svc_crosshairangle* thisptr, bf_read& msg);
};