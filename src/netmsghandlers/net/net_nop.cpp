#include "../../leychan.h"

#include "net_nop.h"

bool net_nop::Register(leychan* chan)
{
	void* voidedfn = static_cast<void*>(&net_nop::ParseMessage);

	leychan::netcallbackfn fn = static_cast<leychan::netcallbackfn>(voidedfn);

	return chan->RegisterMessageHandler(this->GetMsgType(), this, fn);
}

bool net_nop::ParseMessage(leychan* chan, net_nop* thisptr, bf_read& msg)
{

	return true;
}