#include "../../valve/buf.h"

#include "../../leychan.h"
#include "svc_sounds.h"

bool svc_sounds::Register(leychan* chan)
{
	void* voidedfn = static_cast<void*>(&svc_sounds::ParseMessage);

	leychan::netcallbackfn fn = static_cast<leychan::netcallbackfn>(voidedfn);

	return chan->RegisterMessageHandler(this->GetMsgType(), this, fn);
}

bool svc_sounds::ParseMessage(leychan* chan, svc_sounds* thisptr, bf_read& msg)
{
	int reliable = msg.ReadOneBit();

	int num = 1;
	int bits = 0;

	if (reliable == 0)
	{
		num = msg.ReadUBitLong(8);
		bits = msg.ReadUBitLong(16);
	}
	else {
		bits = msg.ReadUBitLong(8);
	}

	if (bits < 1)
		return true;

	char* data = new char[bits];
	msg.ReadBits(data, bits);

	delete[] data;

	// printf("Received svc_Sounds, reliable: %i, bits: %i, num: %i\n", reliable, bits, num);

	return true;
}