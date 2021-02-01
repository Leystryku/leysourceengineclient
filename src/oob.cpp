#pragma once

#include <memory>
#include "valve/buf.h"
#include "../deps/osw/Steamworks.h"
#include "../deps/osw/ISteamUser017.h"

#include "leynet.h"
#include "leychan.h"
#include "steam.h"
#include "datagram.h"
#include "oob.h"


OOB::OOB()
{
	this->udp = 0;
	this->oob = new char[DATAGRAM_SIZE];
	this->netoob = new bf_write;

	Reset();

}

OOB::OOB(leynet_udp* udp, const char* ip, unsigned short port) : OOB()
{
	strcpy(this->ip, ip);
	this->port = port;
	this->udp = udp;

}

void OOB::Reset()
{
	if (this->netoob)
	{
		this->netoob->Reset();
	}

	memset(this->oob, 0, DATAGRAM_SIZE);

	this->netoob->StartWriting(this->oob, DATAGRAM_SIZE, 0);
}

bool OOB::ReceiveQueryPacketReject(leychan* chan, Datagram* datagram, bf_read& recvdata) // A2S_REJECT
{
	recvdata.ReadLong();

	char error[1024];
	recvdata.ReadString(error, 1024);
	printf("Connection refused! [%s]\n", error);

	datagram->Reconnect(chan);
	return true;
}

bool OOB::ReceiveQueryPacketGetChallenge(
	leychan* chan,
	bf_read& recvdata,
	const char* nickname,
	const char* password,
	Steam* steam,
	unsigned long* ourchallenge) // A2A_GETCHALLENGE
{
	chan->connectstep = 3;
	long magicnumber = recvdata.ReadLong();

	unsigned long serverchallenge = recvdata.ReadLong();
	*ourchallenge = recvdata.ReadLong();
	unsigned long authprotocol = recvdata.ReadLong();


	unsigned long steamkey_encryptionsize = recvdata.ReadShort(); // gotta be 0

	unsigned char steamkey_encryptionkey[STEAM_KEYSIZE];
	unsigned char serversteamid[STEAM_KEYSIZE];

	recvdata.ReadBytes(steamkey_encryptionkey, steamkey_encryptionsize);
	recvdata.ReadBytes(serversteamid, sizeof(serversteamid));
	int vacsecured = recvdata.ReadByte();

	printf("Challenge: %lu__%lu|Auth: %x|SKey: %lu|VAC: %x\n", serverchallenge, ourchallenge, authprotocol, steamkey_encryptionsize, vacsecured);

	char connectpkg[700];
	memset(connectpkg, 0, sizeof(connectpkg));

	bf_write writeconnect(connectpkg, sizeof(connectpkg));
	bf_read readsteamid(connectpkg, sizeof(connectpkg));

	writeconnect.WriteLong(-1);
	writeconnect.WriteByte('k');//C2S_CONNECT

	writeconnect.WriteLong(0x18);//protocol ver

	writeconnect.WriteLong(0x03);//auth protocol 0x03 = PROTOCOL_STEAM, 0x02 = PROTOCOL_HASHEDCDKEY, 0x01=PROTOCOL_AUTHCERTIFICATE
	writeconnect.WriteLong(serverchallenge);
	writeconnect.WriteLong(*ourchallenge);

	writeconnect.WriteUBitLong(2729496039, 32);

	writeconnect.WriteString(nickname); //nick
	writeconnect.WriteString(password); // pass
	writeconnect.WriteString("2000"); // game version


	unsigned char steamkey[STEAM_KEYSIZE];
	unsigned int keysize = 0;

	steam->GetSteamUser()->GetAuthSessionTicket(steamkey, STEAM_KEYSIZE, &keysize);

	CSteamID localsid = steam->GetSteamUser()->GetSteamID();

	writeconnect.WriteShort(242);
	unsigned long long steamid64 = localsid.ConvertToUint64();
	writeconnect.WriteLongLong(steamid64);

	if (keysize)
		writeconnect.WriteBytes(steamkey, keysize);

	udp->SendTo(ip, port, connectpkg, writeconnect.GetNumBytesWritten());
	return true;
}

bool OOB::ReceiveQueryPacketConnection(leychan* chan, Datagram* datagram, bf_read& recvdata)
{
	if (chan->connectstep == 0 || chan->connectstep > 4)
		return false;

	chan->connectstep = 4;
	printf("Server is telling us we can connect\n");

	bf_write* senddatabuf = chan->GetSendData();

	senddatabuf->WriteUBitLong(6, 6);
	senddatabuf->WriteByte(2);
	senddatabuf->WriteLong(-1);

	senddatabuf->WriteUBitLong(4, 6);
	senddatabuf->WriteString("VModEnable 1");
	senddatabuf->WriteUBitLong(4, 6);
	senddatabuf->WriteString("vban 0 0 0 0");

	datagram->Send(chan);

	printf("Sent connect packet\n");

	return true;
}

bool OOB::ReceiveQueryPacketIgnore(leychan* chan, bf_read& recvdata)
{
	return true;
}

bool OOB::ReceiveQueryPacket(leychan* chan, Datagram* datagram, Steam* steam, bf_read& recvdata, const char* nickname, const char* password, int connection_less, unsigned long* ourchallenge)
{
	recvdata.ReadLong();

	int header = 0;
	int id = 0;
	int total = 0;
	int number = 0;
	short splitsize = 0;

	if (connection_less == 1)
	{
		header = recvdata.ReadByte();
	}
	else {
		id = recvdata.ReadLong();
		total = recvdata.ReadByte();
		number = recvdata.ReadByte();
		splitsize = recvdata.ReadByte();
	}

	switch (header)
	{
	case '9':
	{
		if (this->ReceiveQueryPacketReject(chan, datagram, recvdata))
		{
			return 1;
		}

		return 0;
	}
	case 'A': // A2A_GETCHALLENGE
	{
		if (this->ReceiveQueryPacketGetChallenge(chan, recvdata, nickname, password, steam, ourchallenge))
		{
			return 1;
		}

		return 0;
	}
	case 'B': // S2C_CONNECTION
	{

		if (this->ReceiveQueryPacketConnection(chan, datagram, recvdata))
		{
			Sleep(3000);
			return 1;
		}

		return 0;
	}

	case 'I':
	{
		return 1;
	}
	default:
	{
		printf("Unknown message received from: %s, header: %i ( %c )\n", ip, header, header);
		return 0;
	}
	}
}

bool OOB::HandleSplitPacket(leychan* chan, bf_read& recvdata, char* netrecbuffer, int msgsize, long* header)
{
	printf("SPLIT\n");

	if (!chan->HandleSplitPacket(netrecbuffer, msgsize, recvdata))
		return false;

	*header = recvdata.ReadLong();
	return true;
}

bool OOB::HandleCompressedPacket(leychan* chan, bf_read& recvdata, char* netrecbuffer, int msgsize)
{
	unsigned int uncompressedSize = msgsize * 16;

	char* tmpbuffer = new char[uncompressedSize];

	memmove(netrecbuffer, netrecbuffer + 4, msgsize + 4);

	leychan::NET_BufferToBufferDecompress(tmpbuffer, uncompressedSize, netrecbuffer, msgsize);

	memcpy(netrecbuffer, tmpbuffer, uncompressedSize);


	recvdata.StartReading(netrecbuffer, uncompressedSize, 0);
	printf("UNCOMPRESSED\n");


	delete[] tmpbuffer;
	tmpbuffer = 0;

	return true;
}

bool OOB::SendRequestChallenge(leychan* chan, long ourchallenge)
{
	char challengepkg[100];

	bf_write writechallenge(challengepkg, sizeof(challengepkg));
	writechallenge.WriteLong(-1);
	writechallenge.WriteByte('q');
	writechallenge.WriteLong(ourchallenge);
	writechallenge.WriteString("0000000000");

	udp->SendTo(ip, port, challengepkg, writechallenge.GetNumBytesWritten());
	chan->connectstep++;
	Sleep(500);
	return 1;
}
