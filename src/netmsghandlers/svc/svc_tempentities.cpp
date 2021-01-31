#include "../../valve/buf.h"

#include "../../leychan.h"
#include "../../vector.h"
#include "svc_tempentities.h"

bool svc_tempentities::Register(leychan* chan)
{
	void* voidedfn = static_cast<void*>(&svc_tempentities::ParseMessage);

	leychan::netcallbackfn fn = static_cast<leychan::netcallbackfn>(voidedfn);

	return chan->RegisterMessageHandler(this->GetMsgType(), this, fn);
}

bool svc_tempentities::ParseMessage(leychan* chan, svc_tempentities* thisptr, bf_read& msg)
{
	int num = msg.ReadUBitLong(8);
	int bits = msg.ReadVarInt32();

	if (bits < 1)
		return true;

	char* data = new char[bits];

	msg.ReadBits(data, bits);

	delete[] data;

	printf("Received svc_TempEntities, num: %i | bits: %i\n", num, bits);

	return true;
}