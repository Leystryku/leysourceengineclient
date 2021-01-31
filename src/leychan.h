#pragma once
//credits to valve for creating CNetChan
//credits to leystryku for assembling the required pieces for a com & updating those to work with modern ob games

#include <vector>

#include "valve/datafragments.h"
#include "valve/subchannel.h"
#include "leychandefs.h"

class bf_read;

class leychan
{
public:



	dataFragments_t					m_ReceiveList[MAX_STREAMS];
	subChannel_s					m_SubChannels[MAX_SUBCHANNELS];
	std::vector<dataFragments_t*>	m_WaitingList[MAX_STREAMS];	// waiting list for reliable data and file transfer

	int m_PacketDrop;
	int m_nInSequenceNr;
	int m_nOutSequenceNrAck;
	int m_nOutReliableState;
	int m_nInReliableState;
	int m_nOutSequenceNr;
	int m_iSignOnState;
	long m_iServerCount;

	unsigned int m_ChallengeNr;
	bool m_bStreamContainsChallenge;

	int ProcessPacketHeader(int msgsize, bf_read& read);
	bool ReadSubChannelData(bf_read& buf, int stream);

	bool CheckReceivingList(int nList);
	void CheckWaitingList(int nList);
	void UncompressFragments(dataFragments_t* data);
	void RemoveHeadInWaitingList(int nList);
	bool NeedsFragments();
	int m_iForceNeedsFrags;

	int HandleSplitPacket(char* netrecbuffer, int& msgsize, bf_read& recvdata);

	void Initialize();

	static unsigned short BufferToShortChecksum(void* pvData, size_t nLength);
	static unsigned short CRC16_ProcessSingleBuffer(unsigned char* data, unsigned int size);
	static bool NET_BufferToBufferDecompress(char* dest, unsigned int& destLen, char* source, unsigned int sourceLen);
	static bool NET_BufferToBufferCompress(char* dest, unsigned int* destLen, char* source, unsigned int sourceLen);
};