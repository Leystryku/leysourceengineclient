#include "../../valve/buf.h"

#include "../../leychan.h"
#include "net_signonstate.h"

bool net_signonstate::Register(leychan* chan)
{
	void* voidedfn = static_cast<void*>(&net_signonstate::ParseMessage);

	leychan::netcallbackfn fn = static_cast<leychan::netcallbackfn>(voidedfn);

	return chan->RegisterMessageHandler(this->GetMsgType(), this, fn);
}

bool net_signonstate::ParseMessage(leychan* chan, net_signonstate* thisptr, bf_read& msg)
{
	int state = msg.ReadByte();
	if (msg.IsOverflowed())
	{
		return false;
	}
	long servercount = msg.ReadLong();

	if (state > 10)
	{
		printf("Invalid net_SignonState!\n");
		return false;
	}

	printf("Received net_SignOnState: %i, count: %i\n", state, servercount);
	chan->SetSignonState(state, servercount);

	return true;
}