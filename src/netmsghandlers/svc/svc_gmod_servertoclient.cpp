#include "../../valve/buf.h"

#include "../../leychan.h"
#include "../../vector.h"
#include "svc_gmod_servertoclient.h"

bool svc_gmod_servertoclient::Register(leychan* chan)
{
	void* voidedfn = static_cast<void*>(&svc_gmod_servertoclient::ParseMessage);

	leychan::netcallbackfn fn = static_cast<leychan::netcallbackfn>(voidedfn);

	return chan->RegisterMessageHandler(this->GetMsgType(), this, fn);
}

bool svc_gmod_servertoclient::ParseMessage(leychan* chan, svc_gmod_servertoclient* thisptr, bf_read& msg)
{
	// TODO: Check whether this is the correct msg structure svc_Gmod_ServerToClient
	int bits = msg.ReadUBitLong(20);
	int type = msg.ReadByte(); // 4= probably server telling about files

	if (bits < 1)
		return true;


	if (bits < 0)
	{
		printf("Received svc_Gmod_ServerToClient || Invalid!\n");

		return true;
	}

	if (type == 4)
	{

		char* data = new char[bits];


		int id = msg.ReadWord();

		int toread = bits - 8 - 16;
		if (toread > 0)
		{
			msg.ReadBits(data, toread);
		}



		printf("Received svc_GMod_ServerToClient, type: %i |  bits: %i  | id: %i \n", type, bits, id);

		delete[] data;

		return true;

	}

	if (type == 3)
	{

		printf("Received svc_GMod_ServerToClient, type: %i |  bits: %i\n", type, bits);

		return true;
	}


	if (type == 0)
	{

		int id = msg.ReadWord();

		char* data = new char[bits];

		int toread = bits - 8 - 16;
		if (toread > 0)
		{
			msg.ReadBits(data, toread);
		}

		delete[] data;

		printf("Received svc_GMod_ServerToClient, type: %i |  bits: %i\n", type, bits);
		return true;
	}


	printf("Received svc_GMod_ServerToClient, type: %i |  bits: %i\n", type, bits);

	return true;
}