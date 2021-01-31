#include "../../valve/buf.h"

#include "../../leychan.h"
#include "svc_createstringtable.h"

bool svc_createstringtable::Register(leychan* chan)
{
	void* voidedfn = static_cast<void*>(&svc_createstringtable::ParseMessage);

	leychan::netcallbackfn fn = static_cast<leychan::netcallbackfn>(voidedfn);

	return chan->RegisterMessageHandler(this->GetMsgType(), this, fn);
}

bool svc_createstringtable::ParseMessage(leychan* chan, svc_createstringtable* thisptr, bf_read& msg)
{
	char name[500];
	msg.ReadString(name, sizeof(name));

	short maxentries = msg.ReadWord();

	unsigned int size = (int)(log2(maxentries) + 1);
	int entries = msg.ReadUBitLong(size);

	int bits = msg.ReadVarInt32();
	int userdata = msg.ReadOneBit();

	if (userdata == 1)
	{
		int userdatasize = msg.ReadUBitLong(12);

		int userdatabits = msg.ReadUBitLong(4);
	}

	int compressed = msg.ReadOneBit();

	if (bits < 1)
		return true;

	char* data = new char[bits];


	msg.ReadBits(data, bits);

	delete[] data;


	printf("Received svc_CreateStringTable, name: %s | maxentries: %i | size: %d | entries: %i | compressed: %i\n", name, maxentries, size, entries, compressed);

	return true;
}