
#include "leynet.h"

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>

#define isalpha IsCharAlpha
#define isupper IsCharUpper
#define isalnum IsCharAlphaNumeric
#define tolower CharLower

char* leynet::HandleError()
{

	int wsaerr = WSAGetLastError();

	if (wsaerr == 10035 && !nonblock)
		return 0;

	if (wsaerr == 0)
		return (char*)"k";

#ifdef DEBUG
	if (customerrorhandling)
	{
		char *errmsg = new char[255];
		sprintf(errmsg, "start failed %i\n", wsaerr);
		return errmsg;
	}

	char errmsg[255];
	sprintf(errmsg, "start failed %i\n", wsaerr);
	printf("errmsg: %s\n", errmsg);
#else
	return (char*)"k";
#endif


	return (char*)"k";
}


char* leynet::SetNonBlocking(bool block)
{
	unsigned long mode = (unsigned long)block;
	ioctlsocket(sock, FIONBIO, &mode);
	nonblock = !block;

	return 0;
}

unsigned short leynet::GetPort()
{
	return uport;
}

//udp
//opening/closing/starting

char* leynet_udp::Start(bool customerr)
{
	if (customerr)
		customerrorhandling = true;

	unsigned short wVersionRequested = MAKEWORD(2, 2);
	WSADATA wsaData;
	int ret = WSAStartup(wVersionRequested, &wsaData);

	if (ret < 0)
		return HandleError();

	return 0;
}

char* leynet_udp::OpenSocket(short port)
{

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	sockaddr_in add;
	add.sin_family = AF_INET;
	add.sin_addr.s_addr = htonl(INADDR_ANY);
	add.sin_port = htons(port);

	int ret = bind(sock, (SOCKADDR*)(&add), sizeof(add));

	if (ret < 0)
		return HandleError();

	uport = port;

	return 0;
}

char* leynet_udp::CloseSocket()
{
	if (sock)
		closesocket(sock);

	sock = 0;
	uport = 0;
	nonblock = 0;

	return 0;
}

//actions

char* leynet_udp::SendTo(const char*ip, short port, const char *buffer, int len)
{

	if (!sock)
		return (char*)"k";

	sockaddr_in add;
	add.sin_family = AF_INET;
	add.sin_port = htons(port);


	inet_pton(AF_INET, ip, &(add.sin_addr));

	int ret = sendto(sock, buffer, len, 0, reinterpret_cast<SOCKADDR *>(&add), sizeof(add));

	if (ret < 0)
		return HandleError();

	return 0;

}

char *leynet_udp::Receive(int*msgsize, char*buffer, int buffersize, char*ip, short recport)
{
	if (!sock)
		return (char*)"k";

	sockaddr_in from;

	int size = sizeof(from);
	int ret = recvfrom(sock, buffer, buffersize, 0, (SOCKADDR*)&from, &size);

	if (ret < 0)
		return HandleError();

	if (ret > buffersize)
		ret = buffersize - 1;

	buffer[ret + 1] = 0;

	*msgsize = ret;

	inet_ntop(AF_INET, &from.sin_addr, ip, INET_ADDRSTRLEN);

	if (recport != -1)
		recport = ntohs(from.sin_port);

	return 0;
}


//tcp
//opening/closing/starting

char* leynet_tcp::Start(bool customerr)
{
	if (customerr)
		customerrorhandling = true;

	unsigned short wVersionRequested = MAKEWORD(2, 2);
	WSADATA wsaData;
	int ret = WSAStartup(wVersionRequested, &wsaData);

	if (ret < 0)
		return HandleError();

	return 0;
}

char* leynet_tcp::Bind(unsigned short bport)
{
	sock = socket(AF_INET, SOCK_STREAM, 0);

	sockaddr_in add;
	add.sin_family = AF_INET;
	add.sin_addr.s_addr = htonl(INADDR_ANY);
	add.sin_port = htons(bport);

	int ret = bind(sock, (SOCKADDR*)&add, sizeof(add));


	if (ret < 0)
	{
		closesocket(sock);
		return HandleError();
	}

	listen(sock, 3);

	return 0;
}

char* leynet_tcp::Listen(char*ip, unsigned short&lport, unsigned int*ssock)
{

	sockaddr_in client;

	int len = sizeof(sockaddr_in);


	unsigned int accepted = (unsigned int)accept(sock, (sockaddr*)&client, &len);

	if (accepted<0)
	{

		return 0;
	}

	*ssock = accepted;

	inet_ntop(AF_INET, &client.sin_addr, ip, INET_ADDRSTRLEN);


	if (lport != 0)
		lport = ntohs(client.sin_port);

	return 0;
}

char* leynet_tcp::OpenConnection(char*addr, unsigned short port)
{

	addrinfo *hints = new addrinfo;
	memset(hints, 0, sizeof(*hints));
	hints->ai_family = AF_INET;
	hints->ai_protocol = IPPROTO_TCP;
	hints->ai_socktype = SOCK_STREAM;

	addrinfo* addrinfo = 0;

	int ret = getaddrinfo(addr, NULL, hints, &addrinfo);

	if (ret < 0 || !addrinfo)
	{
		free(hints);
		return HandleError();
	}

	free(hints);

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	uport = port;

	sockaddr_in add;
	add.sin_family = AF_INET;
	add.sin_port = htons(port);
	add.sin_addr = ((struct sockaddr_in*)addrinfo->ai_addr)->sin_addr;

	freeaddrinfo(addrinfo);

	ret = connect(sock, (SOCKADDR*)(&add), sizeof(add));

	if (ret<0)
	{
		closesocket(sock);
		sock = 0;
		return HandleError();
	}

	strcpy(connectip, addr);

	return 0;
}


char* leynet_tcp::CloseConnection()
{
	if (sock)
		closesocket(sock);

	sock = 0;
	uport = 0;
	nonblock = 0;

	for (int i = 0; i < 25; i++)
		connectip[i] = 0;

	return 0;
}

//actions

char *leynet_tcp::HTTPParseLength(int*msgsize, char*msg)
{
	int size = *msgsize;

	int newsize = 0;

	char *foundlength = strstr(msg, "Content-Length:");

	if (foundlength)
	{
		printf("found length!\n");

		char *start = foundlength + 16;

		int endlen = 1;

		for (endlen; endlen < 15; endlen++)
		{
			if (start[endlen] == '\r'&&start[endlen + 1] == '\n')
				break;
		}

		char contentlength[50];
		memcpy(contentlength, start, endlen);
		contentlength[endlen] = 0;

		newsize += strtol(contentlength, 0, 10);
	}

	for (int i = 0; i < size; i++)
	{
		/*
		if (msg[i] == '\r'&&msg[i + 1] == '\n'&&msg[i + 2] == '\r'&&msg[i + 3] == '\n')
		{
		i = i + 4;
		continue;
		}*/

		if (msg[i] != '\r' || msg[i + 1] != '\n')
		{
			continue;
		}

		if (!isalnum(msg[i + 2]))
			continue;


		bool bfound = false;

		int a = 0;
		for (a = 2; a < 12; a++)
		{
			if (msg[i + a] == '\r'&&msg[i + a + 1] == '\n')
			{
				bfound = true;
				break;
			}

		}

		if (!bfound)
			continue;



		char numstr[20];
		memset(numstr, 0, 20);
		memcpy(numstr, msg + i + 2, a - 2);


		int skipper = strtol(numstr, 0, 16);



		newsize += skipper;
	}

	*msgsize = newsize;

	return 0;
}

char *leynet_tcp::HTTPParse(int* msgsize, char* msg)
{

	int size = *msgsize;

	if (!size)
		return 0;

	char *header = new char[16000];

	for (int i = 0; i < size; i++)
	{
		if (msg[i] == '\r'&&msg[i + 1] == '\n'&&msg[i + 2] == '\r'&&msg[i + 3] == '\n')
		{
			memcpy(header, msg, i + 2);//save header
			header[i + 3] = 0;

			memcpy(msg, msg + i + 2, size - i);//remove header from raw msg
			size = size - i;
			break;
		}
	}

	if (strlen(header) == 0)
	{
		return 0;
	}


	char *foundlength = strstr(header, "Content-Length:");

	int newsize = 0;

	if (foundlength)
	{
		printf("found length!\n");

		char *start = foundlength + 16;

		int endlen = 1;

		for (endlen; endlen < 15; endlen++)
		{
			if (start[endlen] == '\r'&&start[endlen + 1] == '\n')
				break;
		}

		char contentlength[50];
		memcpy(contentlength, start, endlen);
		contentlength[endlen] = 0;

		newsize = strtol(contentlength, 0, 10);
	}


	printf("chunked..\n");


	for (int i = 0; i < size; i++)
	{
		if (msg[i] != '\r' || msg[i + 1] != '\n')
			continue;

		bool bfound = false;

		int a = 0;
		for (a = 1; a < 12; a++)
		{
			if (msg[i + a] == '\r'&&msg[i + a + 1] == '\n')
			{
				bfound = true;
				break;
			}

			if (!isalnum(msg[i + a]))
				break;

		}

		if (!bfound)
			continue;

		char numstr[20] = { 0 };

		memcpy(numstr, msg + i + 2, a - 2);

		int skipper = strtol(numstr, 0, 16);

		memcpy(msg + i, msg + i + a + 1, size - i);

		if (!foundlength)
		{
			newsize += skipper;
		}
	}


	if (msg[0] == '\r'&&msg[1] == '\n')
	{
		memcpy(msg, msg + 2, newsize);
	}

	delete[] header;

	size = newsize;
	*msgsize = size;
	return 0;
}

char* leynet_tcp::GetIP()
{
	return connectip;
}

char* leynet_tcp::HTTPGet(const char *resource)
{
	char request[800];
	sprintf(request, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: Keep-Alive\r\n\r\n", resource, connectip);

	char *ret = Send(request, strlen(request));

	if (ret)
		return ret;

	return 0;
}

char* leynet_tcp::Send(const char *buffer, int len)
{
	if (!sock)
		return (char*)"k";

	int ret = send(sock, buffer, len, 0);

	if (ret < 0)
		return HandleError();

	return 0;

}

char *leynet_tcp::Receive(int* msgsize, char*buffer, int buffersize, TCP_Finished fin)
{

	unsigned int totaldatalen = 0;

	int curbuffer_size = 0xFFFFFFF;
	if (curbuffer_size > buffersize)
		curbuffer_size = buffersize - 3;

	char*curbuffer = new char[curbuffer_size];

	while (true)
	{

		if (buffer && totaldatalen > (unsigned int)buffersize)
			break;

		memset(curbuffer, 0, curbuffer_size);

		int curdatalen = recv(sock, curbuffer, curbuffer_size, 0);
		if (curdatalen > 0)
		{

			if (buffer)
				memcpy(buffer + totaldatalen, curbuffer, curdatalen);

			totaldatalen += curdatalen;
		}

		if (fin)
		{

			bool is_fin = false;

			if (curdatalen <= 0)
			{
				is_fin = fin(totaldatalen, 0, buffer, curbuffer);
			}
			else {
				is_fin = fin(totaldatalen, curdatalen, buffer, curbuffer);
			}

			if (is_fin)
			{

				break;
			}
		}

		if (!fin&&curdatalen<1)
			break;

	}


	delete[] curbuffer;

	if (buffer)
		buffer[totaldatalen + 1] = 0;

	*msgsize = *msgsize + totaldatalen;


	return 0;
}

bool leynet_tcp::TLenFin(unsigned int datalen, unsigned int curdatalen, char*buffer, char*curbuffer)
{

	if (lenfin&&datalen >= lenfin)
	{
		lenfin = 0;
		timefin = 0;
		timefin_s = 0;

		return true;
	}

	if (timefin)
	{
		SYSTEMTIME wintime;
		GetSystemTime(&wintime);

		if (!timefin_s)
		{
			timefin_s = wintime.wSecond;
		}

		if (abs(timefin_s - wintime.wSecond) >= timefin)
		{
			lenfin = 0;
			timefin = 0;
			timefin_s = 0;
			return true;
		}

		return false;
	}

	return true;
}

bool leynet_tcp::THTTPLenFin(unsigned int datalen, unsigned int curdatalen, char*buffer, char*curbuffer)
{

	int penis = curdatalen;

	HTTPParseLength(&penis, curbuffer);

	lenfin += penis;

	printf("DATALEN: %i LENFIN: %i\n", datalen, lenfin);

	if (lenfin&&datalen >= lenfin)
	{
		lenfin = 0;
		timefin = 0;
		timefin_s = 0;

		return true;
	}

	if (timefin)
	{
		SYSTEMTIME wintime;
		GetSystemTime(&wintime);

		if (!timefin_s)
		{
			timefin_s = wintime.wSecond;
		}

		if (abs(timefin_s - wintime.wSecond) >= timefin)
		{
			lenfin = 0;
			timefin = 0;
			timefin_s = 0;
			return true;
		}

		return false;
	}



	return true;
}
