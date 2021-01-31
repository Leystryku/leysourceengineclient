#include "../../valve/buf.h"

#include "../../leychan.h"
#include "net_disconnect.h"

bool net_disconnect::Register(leychan* chan)
{
	void* voidedfn = static_cast<void*>(&net_disconnect::ParseMessage);

	leychan::netcallbackfn fn = static_cast<leychan::netcallbackfn>(voidedfn);

	return chan->RegisterMessageHandler(this->GetMsgType(), this, fn);
}

bool net_disconnect::ParseMessage(leychan* chan, net_disconnect* thisptr, bf_read& msg)
{
	char dcreason[1024];
	msg.ReadString(dcreason, sizeof(dcreason));
	printf("Disconnected: %s\n", dcreason);
	printf("Reconnecting in 100 ms ...");

	chan->Reconnect();
	return true;
}