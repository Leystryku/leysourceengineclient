#pragma once

#ifndef DATAGRAM_SIZE
#define DATAGRAM_SIZE 30000
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



private:
	leynet_udp* udp;


	char* datagram;

	bf_write* netdatagram;

	char ip[30];
	unsigned short port;


};