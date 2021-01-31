#include "../../valve/buf.h"

#include "../../leychan.h"
#include "svc_fixangle.h"

bool svc_fixangle::Register(leychan* chan)
{
	void* voidedfn = static_cast<void*>(&svc_fixangle::ParseMessage);

	leychan::netcallbackfn fn = static_cast<leychan::netcallbackfn>(voidedfn);

	return chan->RegisterMessageHandler(this->GetMsgType(), this, fn);
}

bool svc_fixangle::ParseMessage(leychan* chan, svc_fixangle* thisptr, bf_read& msg)
{
	int relative = msg.ReadOneBit();

	float x = msg.ReadBitAngle(16);
	float y = msg.ReadBitAngle(16);
	float z = msg.ReadBitAngle(16);

	printf("Received svc_FixAngle, x:%f y: %f z: %f | relative: %i\n", x, y, z, relative);

	return true;
}