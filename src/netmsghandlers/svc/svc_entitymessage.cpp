#include "../../valve/buf.h"

#include "../../leychan.h"
#include "../../vector.h"
#include "svc_entitymessage.h"

bool svc_entitymessage::Register(leychan* chan)
{
	void* voidedfn = static_cast<void*>(&svc_entitymessage::ParseMessage);

	leychan::netcallbackfn fn = static_cast<leychan::netcallbackfn>(voidedfn);

	return chan->RegisterMessageHandler(this->GetMsgType(), this, fn);
}

bool svc_entitymessage::ParseMessage(leychan* chan, svc_entitymessage* thisptr, bf_read& msg)
{
	// TODO: This message structure might be likely wrong, if yes fix it
	int ent = msg.ReadUBitLong(MAX_EDICT_BITS);
	int entclass = msg.ReadUBitLong(MAX_SERVER_CLASS_BITS);
	int bits = msg.ReadUBitLong(MAX_ENTITYMESSAGE_BITS);

	if (bits < 1)
		return true;

	char* data = new char[bits];
	msg.ReadBits(data, bits);

	delete[] data;

	printf("Received svc_EntityMessage, ent: %i | class: %i | bits: %i\n", ent, entclass, bits);

	return true;
}