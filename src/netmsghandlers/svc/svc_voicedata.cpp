#include "../../valve/buf.h"

#include "../../leychan.h"
#include "svc_voicedata.h"

bool svc_voicedata::Register(leychan* chan)
{
	void* voidedfn = static_cast<void*>(&svc_voicedata::ParseMessage);

	leychan::netcallbackfn fn = static_cast<leychan::netcallbackfn>(voidedfn);

	return chan->RegisterMessageHandler(this->GetMsgType(), this, fn);
}

bool svc_voicedata::ParseMessage(leychan* chan, svc_voicedata* thisptr, bf_read& msg)
{
	//TODO: check, I am not 100% sure whether we are reading the data correctly here

	int client = msg.ReadByte();
	int proximity = msg.ReadByte();
	int bits = msg.ReadWord();

	if (msg.GetNumBitsLeft() < bits)
		bits = msg.GetNumBitsLeft();

	printf("Received svc_VoiceData, client: %i | proximity: %i | bits: %i\n", client, proximity, bits);



	char* voicedata = new char[(bits + 8) / 8];
	msg.ReadBits(voicedata, bits);

	delete[] voicedata;


	return true;
}