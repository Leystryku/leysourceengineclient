#pragma once
#define STEAMWORKS_CLIENT_INTERFACES

#include "../deps/asyncgetline.h"

#include "valve/buf.h"
#include "valve/checksum_crc.h"
#include "valve/utlbuffer.h"
#include "../deps/osw/Steamworks.h"
#include "../deps/osw/ISteamUser017.h"

#include "leychan.h"
#include "leynet.h"
#include "steam.h"
#include "commandline.h"
#include "datagram.h"
#include "oob.h"

leychan netchan;
leynet_udp net;

Steam* steam = 0;
CommandLine* commandline = 0;
Datagram* datagram = 0;
OOB* oob = 0;

std::string serverip = "";
unsigned short serverport = 27015;
unsigned short clientport = 47015;
std::string nickname = "leysourceengineclient";
std::string password = "";



int last_packet_received = 0;
unsigned long ourchallenge = 0x0B5B1842;

char* netrecbuffer = 0;
int doreceivethink()
{
	if (!netrecbuffer)
	{
		netrecbuffer = new char[DATAGRAM_SIZE];
	}


	int msgsize = 0;
	unsigned short port = 0;
	char charip[25] = { 0 };

	char* worked = net.Receive(&msgsize, &port, charip, netrecbuffer, DATAGRAM_SIZE);

	if (!msgsize)
		return 0;

	if (!strstr(serverip.c_str(), charip))
	{
		printf("ip mismatch\n");
		return 0;
	}

	bf_read recvdata(netrecbuffer, msgsize);
	long header = recvdata.ReadLong();

	int connection_less = 0;

	if (header == NET_HEADER_FLAG_QUERY)
		connection_less = 1;

	if (header == NET_HEADER_FLAG_SPLITPACKET)
	{
		if (!oob->HandleSplitPacket(&netchan, recvdata, netrecbuffer, msgsize, &header))
			return 0;
	}

	if (header == NET_HEADER_FLAG_COMPRESSEDPACKET)
	{
		oob->HandleCompressedPacket(&netchan, recvdata, netrecbuffer, msgsize);
	}


	recvdata.Reset();


	if (connection_less)
	{
		bool success = oob->ReceiveQueryPacket(
			&netchan,
			datagram,
			steam,
			recvdata,
			nickname.c_str(),
			password.c_str(),
			connection_less,
			&ourchallenge);

		if (success)
		{
			return 1;
		}

		return 0;
	}

	if (netchan.connectstep == 4)
	{
		netchan.connectstep = 5;
		printf("Received first ingame packet\n");
	}

	int flags = netchan.ProcessPacketHeader(msgsize, recvdata);

	if (flags == -1)
	{

		printf("Malformed package!\n");

		return 1;
	}


	last_packet_received = clock();

	if (flags & PACKET_FLAG_RELIABLE)
	{
		int i = 0;

		int bit = recvdata.ReadUBitLong(3);
		bit = 1 << bit;

		for (i = 0; i < MAX_STREAMS; i++)
		{

			if (recvdata.ReadOneBit() != 0)
			{
				if (!netchan.ReadSubChannelData(recvdata, i))
				{
					return 1;
				}
			}
		}


		FLIPBIT(netchan.m_nInReliableState, bit);

		for (i = 0; i < MAX_STREAMS; i++)
		{
			if (!netchan.CheckReceivingList(i))
			{
				return 1;
			}
		}
	}



	if (recvdata.GetNumBitsLeft() > 0)
	{
		int proc = netchan.ProcessMessages(recvdata);
	}

	static bool neededfragments = false;

	if (netchan.NeedsFragments() || flags & PACKET_FLAG_TABLES)
	{
		neededfragments = true;
		datagram->RequestFragments(&netchan);
	}

	return 1;
}


int dosendthinkloading()
{

	if (netchan.connectstep <= 3)
	{
		if (netchan.connectstep == 1)
		{
			oob->SendRequestChallenge(&netchan, ourchallenge);
			return 1;
		}

		return 0;
	}

	if (netchan.connectstep <= 7)
	{
		if (netchan.connectstep == 6) // needs to dl the stuff from the subchannels
		{


			datagram->Send(&netchan, true);
			Sleep(1000);
			datagram->Send(&netchan);
			netchan.connectstep++;
			return 1;
		}

		datagram->Send(&netchan);
		return 0;
	}

	if (netchan.connectstep == 8) // need to send clc_ClientInfo
	{
		printf("Sending clc_ClientInfo\n");
		netchan.GetSendData()->WriteUBitLong(8, 6);
		netchan.GetSendData()->WriteLong(netchan.m_iServerCount);
		netchan.GetSendData()->WriteLong(-2039274783);//clc_ClientInfo crc
		netchan.GetSendData()->WriteOneBit(1);//ishltv
		netchan.GetSendData()->WriteLong(1337);
		netchan.GetSendData()->WriteUBitLong(0, 21);

		datagram->Send(&netchan);
		Sleep(300);
		netchan.connectstep++;
		for (int i = 3; i <= 6; i++)
		{
			printf("Sending SignonState %i\n", i);
			netchan.GetSendData()->WriteUBitLong(6, 6);
			netchan.GetSendData()->WriteByte(i);
			netchan.GetSendData()->WriteLong(netchan.m_iServerCount);
			datagram->Send(&netchan);
			Sleep(300);
			netchan.connectstep++;
		}

		doreceivethink();
		datagram->Send(&netchan);//netchan is volatile without this for some reason

		return 1;
	}

	if (netchan.connectstep <= 13)
	{
		netchan.connectstep = 0;
	}

	return 0;
}


int dosendthinkingame(long recdiff, bool* lastrecdiff)
{
	if (netchan.m_nInSequenceNr < 130)
	{
		datagram->Send(&netchan);//netchan is volatile without this for some reason
		return 0;
	}

	if (!netchan.connectstep && !netchan.NeedsFragments() && recdiff >= 15 && !*lastrecdiff)
	{
		datagram->Reset();
		netchan.GetSendData()->WriteOneBit(0);
		netchan.GetSendData()->WriteOneBit(0);
		datagram->Send(&netchan, true);
		*lastrecdiff = true;
	}
	else {
		*lastrecdiff = false;
	}

	if (netchan.m_nInSequenceNr < 130)
	{
		datagram->Send(&netchan);//netchan is volatile without this for some reason
		return 1;
	}

	static int skips = 10;

	if (skips)
	{
		skips--;
	}
	else {
		bf_write* senddatabuf = netchan.GetSendData();

		senddatabuf->WriteUBitLong(3, 6);
		senddatabuf->WriteLong(netchan.tickData.net_tick);
		senddatabuf->WriteUBitLong(netchan.tickData.net_hostframetime, 16);
		senddatabuf->WriteUBitLong(netchan.tickData.net_hostframedeviation, 16);

		skips = 10;
	}

	if (netchan.GetSendData()->GetNumBytesWritten() > 0)
	{
		datagram->Send(&netchan);
		return 1;
	}

	return 0;
}

int dosendthink()
{

	static bool lastrecdiff = false;

	clock_t diffticks = last_packet_received - clock();
	clock_t diffms = (diffticks) / (CLOCKS_PER_SEC / 1000);
	long recdiff = (long)diffms;

	if (recdiff > 20000)
	{
		datagram->Reconnect(&netchan);
		return 1;
	}

	if (netchan.connectstep)
	{
		return dosendthinkloading();
	}


	return dosendthinkingame(recdiff, &lastrecdiff);
}

void donamethink()
{
	int step = netchan.connectstep;
	char buf[0xFF];

	if (step != 0)
	{
		sprintf(buf, "LeySourceEngineClient - Connecting [%d]", step);
		SetConsoleTitleA(buf);
	}
	else {
		SetConsoleTitle(L"LeySourceEngineClient - Ingame");
	}
}

extern void docmdthink(Datagram* datagram, leychan* netchan); // TODO: refactor this to be a class

int main(int argc, const char* argv[])
{
	if (argc <= 1)
	{
		printf("Args: -serverip ip -serverport port -clientport clport -nickname name -password pass");
		return 1;
	}

	commandline = new CommandLine;
	steam = new Steam;
	commandline->InitParams(argv, argc);

	serverip = commandline->GetParameterString("-serverip");
	serverport = commandline->GetParameterNumber("-serverport");
	clientport = commandline->GetParameterNumber("-clientport", true, 47015);
	nickname = commandline->GetParameterString("-nickname", true, "leysourceengineclient");
	password = commandline->GetParameterString("-password", true, "");

	int err = steam->Initiate();

	if (err)
	{
		printf("Failed to initiate Steam: %d\n", err);
	}


	printf(
		"Connecting to %s:%i | client port: %hu | Nick: %s | Pass: %s\n",
		serverip.c_str(),
		serverport,
		clientport,
		nickname.c_str(),
		password.c_str()
	);

	netchan.Initialize();


	net.Start();
	net.OpenSocket(clientport);
	net.SetNonBlocking(true);


	datagram = new Datagram(&net, serverip.c_str(), serverport);
	oob = new OOB(&net, serverip.c_str(), serverport);

	while (true)
	{
		_sleep(1);
		donamethink();
		if (dosendthink())
		{
			continue;
		}

		doreceivethink();
		docmdthink(datagram, &netchan);

	}

	net.CloseSocket();

	return 0;
}
