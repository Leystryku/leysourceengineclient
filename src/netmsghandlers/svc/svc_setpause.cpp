#include "../../valve/buf.h"

#include "../../leychan.h"
#include "svc_setpause.h"

bool svc_setpause::Register(leychan* chan)
{
	void* voidedfn = static_cast<void*>(&svc_setpause::ParseMessage);

	leychan::netcallbackfn fn = static_cast<leychan::netcallbackfn>(voidedfn);

	return chan->RegisterMessageHandler(this->GetMsgType(), this, fn);
}

bool svc_setpause::ParseMessage(leychan* chan, svc_setpause* thisptr, bf_read& msg)
{
	int state = msg.ReadOneBit();
	printf("Received svc_SetPause, state: %i\n", state);

	return true;
}