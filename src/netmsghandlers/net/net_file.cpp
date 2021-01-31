#include "../../valve/buf.h"

#include "../../leychan.h"
#include "net_file.h"

bool net_file::Register(leychan* chan)
{
	void* voidedfn = static_cast<void*>(&net_file::ParseMessage);

	leychan::netcallbackfn fn = static_cast<leychan::netcallbackfn>(voidedfn);

	return chan->RegisterMessageHandler(this->GetMsgType(), this, fn);
}

bool net_file::ParseMessage(leychan* chan, net_file* thisptr, bf_read& msg)
{
	long transferid = msg.ReadUBitLong(32);
	char filename[255];
	msg.ReadString(filename, sizeof(filename));
	bool requested = (bool)(msg.ReadOneBit() == 1);

	if (requested)
		printf("net_File: Server requested file: %s::%i\n", filename, transferid);
	else
		printf("net_File: Server is not sending file: %s::%i\n", filename, transferid);

	return true;
}