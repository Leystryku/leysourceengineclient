#pragma once

#ifndef DATAGRAM_SIZE
#define DATAGRAM_SIZE 65535 * 2
#endif

#ifndef SENDDATA_SIZE
#define SENDDATA_SIZE DATAGRAM_SIZE / 2
#endif

class leychan;
class leynet_udp;

class Datagram
{
private:
	Datagram();
public:
	Datagram(leynet_udp* udp, const char* ip, unsigned short port);

	bool Reset();
	bool Disconnect(leychan* chan);
	bool Reconnect(leychan* chan);
	bool Send(leychan* chan, bool subchans = false);
	bool RequestFragments(leychan* chan);
	void GenerateLeyFile(leychan* chan, const char* filename, const char* content);

	bf_write& GetSendData()
	{
		return senddata;
	}

private:
	leynet_udp* udp = 0;

	char* netsendbuffer = 0;
	char* datagram = 0;

	bf_write senddata;
	bf_write netdatagram;

	char ip[30];
	unsigned short port;


};