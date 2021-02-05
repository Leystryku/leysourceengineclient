#include "../../valve/buf.h"

#include "../../leychan.h"
#include "../../vector.h"
#include "svc_usermessage.h"

bool svc_usermessage::Register(leychan* chan)
{
	void* voidedfn = static_cast<void*>(&svc_usermessage::ParseMessage);

	leychan::netcallbackfn fn = static_cast<leychan::netcallbackfn>(voidedfn);

	return chan->RegisterMessageHandler(this->GetMsgType(), this, fn);
}

bool svc_usermessage::ParseMessage(leychan* chan, svc_usermessage* thisptr, bf_read& msg)
{
	//TODO: Check that we are reading everything in svc_UserMessage correctly
	int msgtype = msg.ReadByte();
	int bits = msg.ReadUBitLong(MAX_USERMESSAGE_BITS);

	if (bits < 1)
		return true;

	char* data = new char[bits];

	msg.ReadBits(data, bits);

	bf_read userMsg(data, bits);

	if (msgtype == 3)
	{


		int client = userMsg.ReadByte();
		//int bWantsChat = userMsg.ReadByte();
		char readstr[MAX_USER_MSG_DATA];
		userMsg.ReadString(readstr, sizeof(readstr));

		int something3 = userMsg.ReadByte(); //idk, sometimes 1, might be bchat
		int something4 = userMsg.ReadByte(); //seems to be 1 when teamchatting
		int something5 = userMsg.ReadByte(); //idk, sometimes 1

		printf("Chat message: %i:%s __ %i\n", client, readstr, userMsg.GetNumBytesLeft());

		delete[] data;
		return true;
	}

	if (msgtype == 5)
	{
		userMsg.ReadByte();
		char readstr[MAX_USER_MSG_DATA];
		userMsg.ReadString(readstr, sizeof(readstr));

		printf("umsg print: %s\n", readstr);
	}

	if (msgtype == 44)//nwvar
	{
		int a = userMsg.ReadUBitLong(32);
		int b = userMsg.ReadByte();

		char str[255];
		userMsg.ReadString(str, sizeof(str));

		char str2[255];
		userMsg.ReadString(str2, sizeof(str2));

		//printf("a:%i b: %i name: %s str2: %s | left: %i\n", a, b, str, str2,  userMsg.GetNumBytesLeft());

		delete[] data;
		return true;
	}

	delete[] data;

	printf("Received svc_UserMessage, type: %i | bits: %i\n", msgtype, bits);

	return true;
}