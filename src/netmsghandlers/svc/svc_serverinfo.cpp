#include "../../valve/buf.h"

#include "../../leychan.h"
#include "svc_serverinfo.h"

bool svc_serverinfo::Register(leychan* chan)
{
	void* voidedfn = static_cast<void*>(&svc_serverinfo::ParseMessage);

	leychan::netcallbackfn fn = static_cast<leychan::netcallbackfn>(voidedfn);

	return chan->RegisterMessageHandler(this->GetMsgType(), this, fn);
}

bool svc_serverinfo::ParseMessage(leychan* chan, svc_serverinfo* thisptr, bf_read& msg)
{

	unsigned short protoversion = msg.ReadShort();
	long servercount = msg.ReadLong();

	bool srctv = (bool)(msg.ReadOneBit() == 1);
	bool dedicated = (bool)(msg.ReadOneBit() == 1);
	long crc = msg.ReadLong();
	short maxclasses = msg.ReadWord();
	char mapmd5[16];
	msg.ReadBytes(mapmd5, 16);

	char players = msg.ReadByte();
	char maxplayers = msg.ReadByte();
	float tickinterval = msg.ReadFloat();
	char platform = msg.ReadChar();

	char gamedir[255];
	char levelname[255];
	char skyname[255];
	char hostname[255];
	char loadingurl[255];
	char gamemode[255];

	if (msg.IsOverflowed())
		return false;

	msg.ReadString(gamedir, sizeof(gamedir));
	msg.ReadString(levelname, sizeof(levelname));
	msg.ReadString(skyname, sizeof(skyname));
	msg.ReadString(hostname, sizeof(hostname));
	msg.ReadString(loadingurl, sizeof(loadingurl));
	msg.ReadString(gamemode, sizeof(gamemode));

	if (msg.IsOverflowed())
		return false;

	printf("ServerInfo, players: %lu/%lu | map: %s | name: %s | gm: %s | count: %i | left: %i\n", players, maxplayers, levelname, hostname, gamemode, servercount, msg.GetNumBitsLeft());

	chan->ProcessServerInfo(protoversion, servercount);

	return true;
}