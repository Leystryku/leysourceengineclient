#pragma once

#include <memory>
#include "valve/buf.h"

#include "leynet.h"
#include "leychan.h"
#include "datagram.h"


Datagram::Datagram()
{
	this->udp = 0;
	this->datagram = new char[DATAGRAM_SIZE];
	this->netdatagram = new bf_write;
	Reset();
}

Datagram::Datagram(leynet_udp* udp, const char* ip, unsigned short port) : Datagram()
{
	strcpy(this->ip, ip);
	this->port = port;
	this->udp = udp;

}

bool Datagram::Reset()
{

	if (this->netdatagram)
	{
		this->netdatagram->Reset();
	}

	memset(this->datagram, 0, DATAGRAM_SIZE);

	this->netdatagram->StartWriting(this->datagram, DATAGRAM_SIZE, 0);

	return true;
}

bool Datagram::Disconnect(leychan* chan)
{
	Reset();

	chan->GetSendData()->WriteUBitLong(1, 6);
	chan->GetSendData()->WriteString("Disconnect by User.");

	Send(chan);

	return true;
}

bool Datagram::Reconnect(leychan* chan)
{
	_sleep(500);
	printf("Reconnecting..\n");

	chan->Reset();
	Reset();

	return true;
}

bool Datagram::Send(leychan* chan, bool subchans)
{
	if (chan->GetSendData()->GetNumBytesWritten() == 0)
	{
		return false;
	}

	unsigned char flags = 0;

	netdatagram->WriteLong(chan->m_nOutSequenceNr); // outgoing sequence
	netdatagram->WriteLong(chan->m_nInSequenceNr); // incoming sequence

	bf_write flagpos = *netdatagram;

	netdatagram->WriteByte(0); // flags
	netdatagram->WriteWord(0); // crc16

	int nCheckSumStart = netdatagram->GetNumBytesWritten();



	netdatagram->WriteByte(chan->m_nInReliableState);


	if (subchans)
	{
		flags |= PACKET_FLAG_RELIABLE;
	}



	netdatagram->WriteBits(chan->GetSendData()->GetData(), chan->GetSendData()->GetNumBitsWritten()); // Data

	int nMinRoutablePayload = MIN_ROUTABLE_PAYLOAD;


	while ((netdatagram->GetNumBytesWritten() < MIN_ROUTABLE_PAYLOAD && netdatagram->GetNumBitsWritten() % 8 != 0))
	{
		netdatagram->WriteUBitLong(0, NETMSG_TYPE_BITS);
	}

	flagpos.WriteByte(flags);

	void* pvData = datagram + nCheckSumStart;

	if (pvData)
	{


		int nCheckSumBytes = netdatagram->GetNumBytesWritten() - nCheckSumStart;
		if (nCheckSumBytes > 0)
		{

			unsigned short usCheckSum = leychan::BufferToShortChecksum(pvData, nCheckSumBytes);
			flagpos.WriteUBitLong(usCheckSum, 16);

			chan->m_nOutSequenceNr++;

			this->udp->SendTo(this->ip, this->port, datagram, netdatagram->GetNumBytesWritten());
		}
	}

	this->Reset();
	chan->GetSendData()->Reset();

	return true;
}

bool Datagram::RequestFragments(leychan* chan)
{
	Reset();


	chan->GetSendData()->WriteOneBit(0);
	chan->GetSendData()->WriteOneBit(0);

	Send(chan, true);

	return true;
}

void Datagram::GenerateLeyFile(leychan* chan, const char* filename, const char* content)
{
	printf("OUTGOING: %i\n", chan->m_nOutSequenceNr);


	int nCheckSumStart = chan->GetSendData()->GetNumBytesWritten();

	chan->GetSendData()->WriteUBitLong(chan->m_nInReliableState, 3);//m_nInReliableState
	chan->GetSendData()->WriteOneBit(1);//subchannel
	chan->GetSendData()->WriteOneBit(1); // data follows( if 0 = singleblock )
	chan->GetSendData()->WriteUBitLong(0, MAX_FILE_SIZE_BITS - FRAGMENT_BITS);// startFragment
	chan->GetSendData()->WriteUBitLong(BYTES2FRAGMENTS(strlen(content)), 3);//number of fragments
	chan->GetSendData()->WriteOneBit(1);//is it a file?

	chan->GetSendData()->WriteUBitLong(0xFF, 32);//transferid

	chan->GetSendData()->WriteString(filename);//filename
	chan->GetSendData()->WriteOneBit(0);//compressed?
	chan->GetSendData()->WriteUBitLong(strlen(content), MAX_FILE_SIZE_BITS);//number of bytes of the file
	chan->GetSendData()->WriteString(content);

}