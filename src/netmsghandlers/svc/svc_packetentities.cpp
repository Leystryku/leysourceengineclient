#include "../../valve/buf.h"

#include "../../leychan.h"
#include "../../vector.h"
#include "svc_packetentities.h"

bool svc_packetentities::Register(leychan* chan)
{
	void* voidedfn = static_cast<void*>(&svc_packetentities::ParseMessage);

	leychan::netcallbackfn fn = static_cast<leychan::netcallbackfn>(voidedfn);

	return chan->RegisterMessageHandler(this->GetMsgType(), this, fn);
}

bool svc_packetentities::ParseMessage(leychan* chan, svc_packetentities* thisptr, bf_read& msg)
{
	// TODO: This message structure is most likely wrong, fix it
	int max = msg.ReadUBitLong(MAX_EDICT_BITS);

	int isdelta = msg.ReadOneBit();

	int delta = -1;

	if (isdelta)
	{
		delta = msg.ReadLong();
	}

	int baseline = msg.ReadUBitLong(1);
	int changed = msg.ReadUBitLong(MAX_EDICT_BITS);
	int bits = msg.ReadUBitLong(DELTASIZE_BITS);

	int updatebaseline = msg.ReadOneBit();

	int bytes = bits / 8;

	if (bits < 1)
		return true;

	char* data = new char[bytes + 10];
	msg.ReadBits(data, bits);

	delete[] data;

	printf("Received svc_PacketEntities | isdelta: %i | line: %i | changed: %i | bits: %i | update: %i\n", isdelta, baseline, changed, bits, updatebaseline);


	return true;
}