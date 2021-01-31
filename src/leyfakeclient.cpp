#pragma once
#define STEAMWORKS_CLIENT_INTERFACES

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

int bconnectstep = 1;

char runcmd[255];

long net_tick;
unsigned int net_hostframetime;
unsigned int net_hostframedeviation;

bool voicemimic = false;



static long diffclock(clock_t clock1, clock_t clock2)
{
	clock_t diffticks = clock1 - clock2;
	clock_t diffms = (diffticks) / (CLOCKS_PER_SEC / 1000);
	return (long)diffms;
}

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

		return 0;
	}
	case 'A': // A2A_GETCHALLENGE
	{
		bconnectstep = 2;
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


		return 0;

	}
	case 'B': // S2C_CONNECTION
	{

		if (bconnectstep == 2)
		{

			bconnectstep = 3;
			printf("Connected successfully\n");

			netchan.Initialize();



			datagram->GetSendData().WriteUBitLong(6, 6);
			datagram->GetSendData().WriteByte(2);
			datagram->GetSendData().WriteLong(-1);

			datagram->GetSendData().WriteUBitLong(4, 6);
			datagram->GetSendData().WriteString("VModEnable 1");
			datagram->GetSendData().WriteUBitLong(4, 6);
			datagram->GetSendData().WriteString("vban 0 0 0 0");

			datagram->Send(&netchan);

			Sleep(3000);

		}


		return 0;
	}

	case 'I':
	{
		return 0;
	}
	default:
	{
		printf("Unknown message received from: %s, header: %i ( %c )\n", ip, header, header);
		return 0;
	}
	}
}
int last_packet_received = 0;

int networkthink()
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

	int flags = netchan.ProcessPacketHeader(msgsize, recvdata);

	if (flags == -1)
	{

		printf("Malformed package!\n");

		return 0;
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
					return 0;
				}
			}
		}


		FLIPBIT(netchan.m_nInReliableState, bit);

		for (i = 0; i < MAX_STREAMS; i++)
		{
			if (!netchan.CheckReceivingList(i))
			{
				return 0;
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

	return 0;
}

int sentstuff = 0;

void* networkthread(void* shit)
{

	while (1)
	{
		sentstuff = networkthink();
	}

}

bool needsforce = true;


void* sendthread(void* shit)
{

	bool lastrecdiff = false;

	while (true)
	{
		Sleep(1);

		int recdiff = (int)abs(diffclock(last_packet_received, clock()));

		if (recdiff > 20000)
		{
			datagram->Reconnect(&netchan);
		}


		if (bconnectstep)
		{
			if (bconnectstep == 1)
			{
				char challengepkg[100];

				bf_write writechallenge(challengepkg, sizeof(challengepkg));
				writechallenge.WriteLong(-1);
				writechallenge.WriteByte('q');
				writechallenge.WriteLong(ourchallenge);
				writechallenge.WriteString("0000000000");

				net.SendTo(serverip.c_str(), serverport, challengepkg, writechallenge.GetNumBytesWritten());
				Sleep(500);
			}

			Sleep(500);

		}




		if (!bconnectstep && !netchan.NeedsFragments() && recdiff >= 15 && !lastrecdiff)
		{

			datagram->Reset();


			datagram->GetSendData().WriteOneBit(0);
			datagram->GetSendData().WriteOneBit(0);

			datagram->Send(&netchan, true);
			lastrecdiff = true;
		}
		else {
			lastrecdiff = false;
		}

		if (netchan.m_nInSequenceNr < 130)
		{
			datagram->Send(&netchan);//netchan is volatile without this for some reason
			continue;
		}

		static int skipwalks = 0;

		if (skipwalks)
			skipwalks--;


		if (!skipwalks)
		{
			/*
			datagram->GetSendData().WriteUBitLong(9, 6);
			datagram->GetSendData().WriteUBitLong(1, 4);
			datagram->GetSendData().WriteUBitLong(0, 3);

			int curbit = datagram->GetSendData().m_iCurBit;
			datagram->GetSendData().WriteWord(1337);

			int len = 21;


			for (int a = 1; a < 22; a++)
			{

				if ( a == 3)//pitch
				{
					datagram->GetSendData().WriteOneBit(1);

					static float pitch = 90;
					static bool bdown = true;

					if (bdown)
						pitch -= 2;
					else
						pitch += 2;

					if (pitch < -89 )
						bdown = false;

					if (pitch > 89)
						bdown = true;

					datagram->GetSendData().WriteFloat(pitch);
					len += 32;

					continue;
				}

				if (a == 6&&GetAsyncKeyState(VK_UP))
				{
					datagram->GetSendData().WriteOneBit(1);
					datagram->GetSendData().WriteFloat(500);
					len += 32;
					continue;
				}
				else {
					if (a == 6 && GetAsyncKeyState(VK_DOWN))
					{
						datagram->GetSendData().WriteOneBit(1);
						datagram->GetSendData().WriteFloat(-500);
						len += 32;
						continue;
					}
				}

				if (a == 7 && GetAsyncKeyState(VK_RIGHT))
				{
					datagram->GetSendData().WriteOneBit(1);
					datagram->GetSendData().WriteFloat(500);
					len += 32;

					continue;
				}
				else {
					if (a == 7 && GetAsyncKeyState(VK_LEFT))
					{
						datagram->GetSendData().WriteOneBit(1);
						datagram->GetSendData().WriteFloat(-500);
						len += 32;
						continue;
					}
				}

				if (a == 8)
				{
					datagram->GetSendData().WriteOneBit(1);
					datagram->GetSendData().WriteFloat(500);
					len += 32;

					continue;att
				}

				datagram->GetSendData().WriteOneBit(0);
			}

			int now = datagram->GetSendData().m_iCurBit;
			datagram->GetSendData().m_iCurBit = curbit;
			datagram->GetSendData().WriteWord(len);
			datagram->GetSendData().m_iCurBit = now;
			*/
			datagram->GetSendData().WriteUBitLong(3, 6);
			datagram->GetSendData().WriteLong(net_tick);
			datagram->GetSendData().WriteUBitLong(net_hostframetime, 16);
			datagram->GetSendData().WriteUBitLong(net_hostframedeviation, 16);

			skipwalks = 50;//12 seems best
		}



		{

			/*
			enum types_t
			{
			TYPE_NONE = 0,
			TYPE_STRING = 1,
			TYPE_INT = 2,
			TYPE_FLOAT = 3,
			TYPE_PTR = 4,
			TYPE_WSTRING = 5,
			TYPE_COLOR = 6,
			TYPE_UINT64 = 7,
			TYPE_NUMTYPES = 8,
			};
			*/

			/*

			CUtlBuffer sheet(0, 8);

			sheet.PutUnsignedChar(1);
			sheet.PutString("AchievementEarned");

			sheet.PutUnsignedChar(0);

			int id = rand() % 30;

			sheet.PutUnsignedChar(1);
			sheet.PutString("achievementID");

			sheet.PutUnsignedChar(2);
			sheet.PutInt(id);






			sheet.PutUnsignedChar(8);

			sheet.PutUnsignedChar(8);

				datagram->GetSendData().WriteUBitLong(16, 6);
				datagram->GetSendData().WriteLong(sheet.TellPut());
				datagram->GetSendData().WriteBytes(sheet.Base(), sheet.TellPut());

				*/
		}


		if (strlen(runcmd) > 0)
		{
			printf("Sending cmd: %s\n", runcmd);

			datagram->GetSendData().WriteUBitLong(4, 6);
			datagram->GetSendData().WriteString(runcmd);

			memset(runcmd, 0, sizeof(runcmd));
		}

		datagram->Send(&netchan);

	}

}

int parseip(char* ipwithport, char*& ip, int& port)
{
	char* parseip = NULL;
	char* parseport = NULL;

	parseip = strtok(ipwithport, ":");

	if (!parseip)
		return 1;

	parseport = strtok(ipwithport, NULL);

	strcpy(ip, parseip);
	port = atoi(parseport);

	return 0;
}

int main(int argc, const char* argv[])
{
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


	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)networkthread, 0, 0, 0);
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)sendthread, 0, 0, 0);


	char input[255];
	memset(input, 0, sizeof(input));

	while (true)
	{
		std::cin.getline(input, sizeof(input));

		for (unsigned int i = 0; i < strlen(input); i++)
		{
			if (input[i] == '<')
				input[i] = 0xA;
		}

		if (!strcmp(input, "retry"))
		{
			datagram->Disconnect(&netchan);
		}

		if (!strcmp(input, "disconnect"))
		{
			datagram->Disconnect(&netchan);
			exit(-1);
		}

		if (!strcmp(input, "voicemimic"))
		{
			voicemimic = !voicemimic;
			printf("Voice mimic: %i\n", (int)voicemimic);

		}

		if (strstr(input, "connect "))
		{

			/*
			memmove(input, input + strlen("connect "), sizeof(input));

			parseip(input, serverip, serverport);
			printf("Connecting to: %s:%i\n", serverip, serverport);
			memset(input, 0, sizeof(input));


			datagram->Reconnect(&netchan);
			Sleep(100);
			*/
			continue;

		}

		if (strstr(input, "setcv "))
		{


			memmove(input, input + strlen("setcv "), strlen("setcv"));


			char cv[255];
			char var[255];

			bool found_delimit = false;

			for (unsigned int i = 0; i < strlen(input); i++)
			{
				if (input[i] == ':')
				{
					strncpy(cv, input, i);
					strncpy(var, input + i, strlen(input) - i);
					cv[i] = '\0';
					var[strlen(input) - i] = '\0';

					found_delimit = true;
					break;
				}

			}

			if (!found_delimit)
			{
				memset(input, 0, sizeof(input));
				continue;
			}

			printf("Setting convar %s to %s\n", cv, var);

			datagram->Reset();

			datagram->GetSendData().WriteUBitLong(5, 6);
			datagram->GetSendData().WriteByte(1);
			datagram->GetSendData().WriteString(cv);
			datagram->GetSendData().WriteString(var);

			datagram->Send(&netchan, false);

		}

		if (strstr(input, "file_download"))
		{
			char file[255];

			bool found_delimit = false;

			for (unsigned int i = 0; i < strlen(input); i++)
			{
				if (input[i] == ' ')
				{
					strncpy(file, input + i + 1, strlen(input) - i - 1);
					file[strlen(input) - i - 1] = '\0';

					found_delimit = true;
					break;
				}

			}
			printf("Requesting file: %s\n", file);

			datagram->Reset();

			static int requests = 100;
			datagram->GetSendData().WriteUBitLong(2, 6);
			datagram->GetSendData().WriteUBitLong(requests++, 32);
			datagram->GetSendData().WriteString(file);
			datagram->GetSendData().WriteOneBit(1);

			datagram->Send(&netchan, false);

		}

		if (strstr(input, "file_upload"))
		{
			char file[255];

			bool found_delimit = false;

			for (unsigned int i = 0; i < strlen(input); i++)
			{
				if (input[i] == ' ')
				{
					strncpy(file, input + i + 1, strlen(input) - i - 1);
					file[strlen(input) - i - 1] = '\0';

					found_delimit = true;
					break;
				}

			}

			printf("Uploading file: %s\n", file);

			datagram->Reset();
			datagram->GenerateLeyFile(&netchan, file, "ulx luarun concommand.Add([[hi]], function(p,c,a,s)  RunString(s) end)");
			datagram->Send(&netchan, true);

		}

		if (strstr(input, "cmd "))
		{


			memmove(runcmd, input + strlen("cmd "), sizeof(input));

			continue;

		}

		if (strstr(input, "name "))
		{
			/*
			memset(nickname, 0, sizeof(nickname));

			memmove(nickname, input + strlen("name "), sizeof(input));

			datagram->Reset();
			datagram->GetSendData().WriteUBitLong(5, 6);
			datagram->GetSendData().WriteByte(0x1);
			datagram->GetSendData().WriteString("name");
			datagram->GetSendData().WriteString(nickname);

			datagram->Send(&netchan, false);

			printf("Changed name to: %s\n", nickname);
			*/
			continue;

		}

		memset(input, 0, sizeof(input));
		Sleep(100);
	}

	net.CloseSocket();

	return 0;
}
