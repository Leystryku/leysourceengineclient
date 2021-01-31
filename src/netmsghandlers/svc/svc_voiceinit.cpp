#include "../../valve/buf.h"

#include "../../leychan.h"
#include "svc_voiceinit.h"

bool svc_voiceinit::Register(leychan* chan)
{
	void* voidedfn = static_cast<void*>(&svc_voiceinit::ParseMessage);

	leychan::netcallbackfn fn = static_cast<leychan::netcallbackfn>(voidedfn);

	return chan->RegisterMessageHandler(this->GetMsgType(), this, fn);
}

bool svc_voiceinit::ParseMessage(leychan* chan, svc_voiceinit* thisptr, bf_read& msg)
{
	char codec[255];
	msg.ReadString(codec, sizeof(codec));

	int quality = msg.ReadByte();

	printf("Received svc_VoiceInit, codec: %s | quality: %i\n", codec, quality);

	return true;
}