#pragma once
#define STEAMWORKS_CLIENT_INTERFACES

#include <fcntl.h>

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

leychan netchan;
leynet_udp net;

Steam* steam = 0;
CommandLine* commandline = 0;
Datagram* datagram = 0;

std::string serverip = "";
unsigned short serverport = 27015;
unsigned short clientport = 47015;
std::string nickname = "leysourceengineclient";
std::string password = "";

char* logon = 0;

char runcmd[255];

long net_tick;
unsigned int net_hostframetime;
unsigned int net_hostframedeviation;

unsigned long ourchallenge = 0x0B5B1842;

unsigned long serverchallenge = 0;
unsigned long authprotocol = 0;
unsigned long steamkey_encryptionsize = 0;
unsigned char steamkey_encryptionkey[STEAM_KEYSIZE];
unsigned char serversteamid[STEAM_KEYSIZE];
int vacsecured = 0;




int HandleConnectionLessPacket(char* ip, short port, int connection_less, bf_read& recvdata)
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
		recvdata.ReadLong();

		char error[1024];
		recvdata.ReadString(error, 1024);
		printf("Connection refused! [%s]\n", error);

		datagram->Reconnect(&netchan);

		return 1;
	}
	case 'A': // A2A_GETCHALLENGE
	{
		netchan.connectstep = 3;
		long magicnumber = recvdata.ReadLong();

		serverchallenge = recvdata.ReadLong();
		ourchallenge = recvdata.ReadLong();
		authprotocol = recvdata.ReadLong();

		steamkey_encryptionsize = recvdata.ReadShort(); // gotta be 0

		recvdata.ReadBytes(steamkey_encryptionkey, steamkey_encryptionsize);
		recvdata.ReadBytes(serversteamid, sizeof(serversteamid));
		vacsecured = recvdata.ReadByte();

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
		writeconnect.WriteLong(ourchallenge);

		writeconnect.WriteUBitLong(2729496039, 32);

		writeconnect.WriteString(nickname.c_str()); //nick
		writeconnect.WriteString(password.c_str()); // pass
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

		net.SendTo(ip, port, connectpkg, writeconnect.GetNumBytesWritten());


		return 1;

	}
	case 'B': // S2C_CONNECTION
	{
		if (netchan.connectstep == 0 || netchan.connectstep > 4)
			return 1;

		netchan.connectstep = 4;
		printf("Server is telling us we can connect\n");

		bf_write* senddatabuf = netchan.GetSendData();

		senddatabuf->WriteUBitLong(6, 6);
		senddatabuf->WriteByte(2);
		senddatabuf->WriteLong(-1);

		senddatabuf->WriteUBitLong(4, 6);
		senddatabuf->WriteString("VModEnable 1");
		senddatabuf->WriteUBitLong(4, 6);
		senddatabuf->WriteString("vban 0 0 0 0");

		datagram->Send(&netchan);

		printf("Sent connect packet\n");
		Sleep(3000);


		return 1;
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
int last_packet_received = 0;

int doreceivethink()
{

	char netrecbuffer[2000];


	int msgsize = 0;
	unsigned short port = 0;
	char charip[25] = { 0 };

	char* worked = net.Receive(&msgsize, &port, charip, netrecbuffer, 2000);

	if (!msgsize)
		return 0;

	if (!strstr(serverip.c_str(), charip))
	{
		printf("ip mismatch\n");
		return 0;
	}

	bf_read recvdata(netrecbuffer, msgsize);
	int header = recvdata.ReadLong();

	int connection_less = 0;

	if (header == NET_HEADER_FLAG_QUERY)
		connection_less = 1;

	if (header == NET_HEADER_FLAG_SPLITPACKET)
	{
		printf("SPLIT\n");

		if (!netchan.HandleSplitPacket(netrecbuffer, msgsize, recvdata))
			return 0;

		header = recvdata.ReadLong();
	}

	if (header == NET_HEADER_FLAG_COMPRESSEDPACKET)
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
	}


	recvdata.Reset();


	if (connection_less)
	{
		return HandleConnectionLessPacket(charip, port, connection_less, recvdata);
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


bool needsforce = true;


static long diffclock(clock_t clock1, clock_t clock2)
{
	clock_t diffticks = clock1 - clock2;
	clock_t diffms = (diffticks) / (CLOCKS_PER_SEC / 1000);
	return (long)diffms;
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


	if (netchan.connectstep <= 3)
	{
		if (netchan.connectstep == 1)
		{
			char challengepkg[100];

			bf_write writechallenge(challengepkg, sizeof(challengepkg));
			writechallenge.WriteLong(-1);
			writechallenge.WriteByte('q');
			writechallenge.WriteLong(ourchallenge);
			writechallenge.WriteString("0000000000");

			net.SendTo(serverip.c_str(), serverport, challengepkg, writechallenge.GetNumBytesWritten());
			netchan.connectstep++;
			Sleep(500);
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

	if (netchan.m_nInSequenceNr < 130)
	{
		datagram->Send(&netchan);//netchan is volatile without this for some reason
		return 0;
	}

	/*
	if (!netchan.connectstep && !netchan.NeedsFragments() && recdiff >= 15 && !lastrecdiff)
	{

		datagram->Reset();


		senddatabuf->WriteOneBit(0);
		senddatabuf->WriteOneBit(0);

		datagram->Send(&netchan, true);
		lastrecdiff = true;
	}
	else {
		lastrecdiff = false;
	}*/

	if (netchan.m_nInSequenceNr < 130)
	{
		datagram->Send(&netchan);//netchan is volatile without this for some reason
		return 0;
	}

	static int skipwalks = 0;

	if (skipwalks)
		skipwalks--;


	if (!skipwalks)
	{
		bf_write* senddatabuf = netchan.GetSendData();

		senddatabuf->WriteUBitLong(3, 6);
		senddatabuf->WriteLong(net_tick);
		senddatabuf->WriteUBitLong(net_hostframetime, 16);
		senddatabuf->WriteUBitLong(net_hostframedeviation, 16);

		skipwalks = 50;
	}


	if (strlen(runcmd) > 0)
	{
		printf("Sending cmd: %s\n", runcmd);

		bf_write* senddatabuf = netchan.GetSendData();

		senddatabuf->WriteUBitLong(4, 6);
		senddatabuf->WriteString(runcmd);

		memset(runcmd, 0, sizeof(runcmd));
	}

	datagram->Send(&netchan);
	return 1;

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
/*
getline_async is from https://stackoverflow.com/questions/16592357/non-blocking-stdgetline-exit-if-no-input
*/
bool getline_async(std::istream& is, std::string& str, char delim = '\n') {

	static std::string lineSoFar;
	char inChar;
	int charsRead = 0;
	bool lineRead = false;
	str = "";

	do {
		charsRead = is.readsome(&inChar, 1);
		if (charsRead == 1) {
			// if the delimiter is read then return the string so far
			if (inChar == delim) {
				str = lineSoFar;
				lineSoFar = "";
				lineRead = true;
			}
			else {  // otherwise add it to the string so far
				lineSoFar.append(1, inChar);
			}
		}
	} while (charsRead != 0 && !lineRead);

	return lineRead;
}

int main(int argc, const char* argv[])
{
	if(argc <= 1)
	{
		printf("Args: -serverip ip -serverport port -clientport clport -nickname name -password pass")
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

	std::string sinput = "";

	while (true)
	{
		_sleep(1);

		donamethink();

		if (dosendthink())
		{
			continue;
		}

		if (doreceivethink())
		{
			continue;
		}
		/*
		if (!getline_async(std::cin, sinput))
			continue;

		for (unsigned int i = 0; i < sinput.length(); i++)
		{
			if (sinput[i] == '<')
				sinput[i] = 0xA;
		}


		const char* input = sinput.c_str();

		if (!strcmp(input, "retry"))
		{
			datagram->Disconnect(&netchan);
			sinput = "";
		}

		if (!strcmp(input, "disconnect"))
		{
			datagram->Disconnect(&netchan);
			exit(-1);
			sinput = "";
		}

		if (strstr(input, "setcv "))
		{
			char* cmd = strtok((char*)input, " ");
			char* cv = strtok(NULL, " ");
			char* var = strtok(NULL, " ");

			printf("Setting convar %s to %s\n", cv, var);

			datagram->Reset();
			bf_write* senddatabuf = netchan.GetSendData();

			senddatabuf->WriteUBitLong(5, 6);
			senddatabuf->WriteByte(1);
			senddatabuf->WriteString(cv);
			senddatabuf->WriteString(var);

			datagram->Send(&netchan, false);
			sinput = "";
		}

		if (strstr(input, "file_download"))
		{
			char* cmd = strtok((char*)input, " ");
			char* file = strtok(NULL, " ");
			printf("Requesting file: %s\n", file);

			datagram->Reset();

			static int requestcount = 100;
			bf_write* senddatabuf = netchan.GetSendData();

			senddatabuf->WriteUBitLong(2, 6);
			senddatabuf->WriteUBitLong(requestcount++, 32);
			senddatabuf->WriteString(file);
			senddatabuf->WriteOneBit(1);

			datagram->Send(&netchan, false);
			sinput = "";
		}

		if (strstr(input, "file_upload"))
		{
			char* cmd = strtok((char*)input, " ");
			char* file = strtok(NULL, " ");

			printf("Uploading file: %s\n", file);

			datagram->Reset();
			datagram->GenerateLeyFile(&netchan, file, "ulx luarun concommand.Add([[hi]], function(p,c,a,s)  RunString(s) end)");
			datagram->Send(&netchan, true);
			sinput = "";
		}

		if (strstr(input, "cmd "))
		{

			char* cmd = strtok((char*)input, " ");
			char* sourcecmd = strtok(NULL, " ");

			bf_write* senddatabuf = netchan.GetSendData();

			senddatabuf->WriteUBitLong(4, 6);
			senddatabuf->WriteString(sourcecmd);
			sinput = "";
			continue;

		}

		if (strstr(input, "name "))
		{
			char* cmd = strtok((char*)input, " ");
			char* nickname = strtok(NULL, " ");

			bf_write* senddatabuf = netchan.GetSendData();

			senddatabuf->WriteUBitLong(5, 6);
			senddatabuf->WriteByte(0x1);
			senddatabuf->WriteString("name");
			senddatabuf->WriteString(nickname);

			datagram->Send(&netchan, false);

			printf("Changing name to: %s\n", nickname);
			sinput = "";
			continue;

		}*/
	}

	net.CloseSocket();

	return 0;
}
