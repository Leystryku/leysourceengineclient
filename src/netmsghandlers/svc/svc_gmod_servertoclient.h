#pragma once

#include "../netmsg_common.h"

class bf_read;
class leychan;

class svc_gmod_servertoclient : public netmsg_common
{
public:
	svc_gmod_servertoclient() : netmsg_common("svc_gmod_servertoclient", 33, sizeof(short)) { }

	bool Register(leychan* chan);
	static bool ParseMessage(leychan* chan, svc_gmod_servertoclient* thisptr, bf_read& msg);
};