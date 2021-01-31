#include "../../valve/buf.h"

#include "../../leychan.h"
#include "../../vector.h"
#include "svc_gameeventlist.h"

bool svc_gameeventlist::Register(leychan* chan)
{
	void* voidedfn = static_cast<void*>(&svc_gameeventlist::ParseMessage);

	leychan::netcallbackfn fn = static_cast<leychan::netcallbackfn>(voidedfn);

	return chan->RegisterMessageHandler(this->GetMsgType(), this, fn);
}

bool svc_gameeventlist::ParseMessage(leychan* chan, svc_gameeventlist* thisptr, bf_read& msg)
{
	int num = msg.ReadUBitLong(9);
	int bits = msg.ReadUBitLong(20);

	if (bits < 1)
		return true;

	char* data = new char[bits];
	msg.ReadBits(data, bits);

	printf("Received svc_GameEventList, num: %i | bits: %i\n", num, bits);

	delete[] data;

	return true;
}