#ifndef LEYNET_H
#define LEYNET_H
#pragma once


class leynet
{
protected:
	bool customerrorhandling;
	bool nonblock;

	int sock;
	unsigned short uport;

	char* HandleError();
public:

	inline void SetSock(int ssock)
	{
		sock = ssock;
	}

	char* SetNonBlocking(bool block);

	//settings

	unsigned short GetPort();

};

class leynet_udp : public leynet
{
private:

public:

	leynet_udp()
	{
		sock = 0;
		uport = 0;
		nonblock = 0;
	}

	~leynet_udp()
	{
		sock = 0;
		uport = 0;
		nonblock = 0;
	}

	//opening/closing/starting

	char *Start(bool customerr = false);

	char *OpenSocket(short port);
	char *CloseSocket();



	//actions

	char *SendTo(const char*ip, short port, const char *buffer, int len);
	char *Receive(int*msgsize, unsigned short*port, char*ip, char*buffer, int buffersize);

};






class leynet_tcp : public leynet
{
private:
	char connectip[25];

public:
	typedef bool(*TCP_Finished)(unsigned int datalen, unsigned int curdatalen, char*buffer, char* curbuffer);

	unsigned int lenfin;
	int timefin;
	int timefin_s;

	leynet_tcp()
	{
		lenfin = 0;
		timefin = 0;
		timefin_s = 0;
		sock = 0;
		uport = 0;
		customerrorhandling = 0;
		nonblock = 0;

		for (int i = 0; i < 25; i++)
			connectip[i] = 0;

	}

	~leynet_tcp()
	{
		lenfin = 0;
		timefin = 0;
		timefin_s = 0;
		sock = 0;
		uport = 0;
		customerrorhandling = 0;
		nonblock = 0;

		for (int i = 0; i < 25; i++)
			connectip[i] = 0;
	}

	//opening/closing/starting

	char* Start(bool customerr = false);

	char *Bind(unsigned short port);
	char* Listen(char*ip, unsigned short&port, unsigned int*sock);
	char* OpenConnection(char*addr, unsigned short port);
	char* CloseConnection();

	//actions
	char* GetIP();
	char* HTTPParse(int* msgsize, char* response);
	char* HTTPParseLength(int* msgsize, char* response);
	char* HTTPGet(const char *resource);
	char* Send(const char *buffer, int len);
	char* Receive(int* msgsize, char*buffer, int buffersize, TCP_Finished fin = 0);


	//Finish functions
	bool TLenFin(unsigned int datalen, unsigned int curdatalen, char*buffer, char* curbuffer);
	bool THTTPLenFin(unsigned int datalen, unsigned int curdatalen, char*buffer, char *curbuffer);

};

#endif
	