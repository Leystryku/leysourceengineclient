#include "../../valve/buf.h"

#include "../../leychan.h"
#include "../../vector.h"
#include "svc_prefetch.h"

bool svc_prefetch::Register(leychan* chan)
{
	void* voidedfn = static_cast<void*>(&svc_prefetch::ParseMessage);

	leychan::netcallbackfn fn = static_cast<leychan::netcallbackfn>(voidedfn);

	return chan->RegisterMessageHandler(this->GetMsgType(), this, fn);
}

bool svc_prefetch::ParseMessage(leychan* chan, svc_prefetch* thisptr, bf_read& msg)
{
	int index = msg.ReadUBitLong(4);

	printf("Received svc_Prefetch, index: %i\n", index);

	return true;
}