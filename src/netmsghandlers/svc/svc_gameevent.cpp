#include "../../valve/buf.h"

#include "../../leychan.h"
#include "../../vector.h"
#include "svc_gameevent.h"

bool svc_gameevent::Register(leychan* chan)
{
	void* voidedfn = static_cast<void*>(&svc_gameevent::ParseMessage);

	leychan::netcallbackfn fn = static_cast<leychan::netcallbackfn>(voidedfn);

	return chan->RegisterMessageHandler(this->GetMsgType(), this, fn);
}

bool svc_gameevent::ParseMessage(leychan* chan, svc_gameevent* thisptr, bf_read& msg)
{
	int bits = msg.ReadUBitLong(11);

	if (bits < 1)
		return true;

	char* data = new char[bits];
	msg.ReadBits(data, bits);

	delete[] data;

	printf("Received svc_GameEvent, bits: %i\n", bits);
	return true;
}