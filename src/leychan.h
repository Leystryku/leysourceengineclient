
#pragma once

#include "stdafx.h"
#include "buf.h"
#include "checksum_crc.h"
#include "clzss.h"

#define PAD_NUMBER(number, boundary) \
	( ((number) + ((boundary)-1)) / (boundary) ) * (boundary)

#define NET_HEADER_FLAG_QUERY					-1
#define NET_HEADER_FLAG_SPLITPACKET				-2
#define NET_HEADER_FLAG_COMPRESSEDPACKET		-3

#define MAX_CUSTOM_FILES 4

#define MAX_TABLES_BITS log2(32)
#define MAX_EDICT_BITS 13
#define MAX_ENTITYMESSAGE_BITS 11
#define MAX_SERVER_CLASS_BITS 9

#define MAX_USERMESSAGE_BITS 11
#define MAX_USER_MSG_DATA 255

// How fast to converge flow estimates
#define FLOW_AVG ( 3.0 / 4.0 )
// Don't compute more often than this
#define FLOW_INTERVAL 0.25


#define NET_FRAMES_BACKUP	64		// must be power of 2
#define NET_FRAMES_MASK		(NET_FRAMES_BACKUP-1)
#define MAX_SUBCHANNELS		8		// we have 8 alternative send&wait bits

#define SUBCHANNEL_FREE		0	// subchannel is free to use
#define SUBCHANNEL_TOSEND	1	// subchannel has data, but not send yet
#define SUBCHANNEL_WAITING	2   // sbuchannel sent data, waiting for ACK
#define SUBCHANNEL_DIRTY	3	// subchannel is marked as dirty during changelevel

#define PROTOCOL_AUTHCERTIFICATE 0x01   // Connection from client is using a WON authenticated certificate
#define PROTOCOL_HASHEDCDKEY     0x02	// Connection from client is using hashed CD key because WON comm. channel was unreachable
#define PROTOCOL_STEAM			 0x03	// Steam certificates
#define PROTOCOL_LASTVALID       0x03    // Last valid protocol
#define STEAM_KEYSIZE				2048  // max size needed to contain a steam authentication key (both server and client)

// each channel packet has 1 byte of FLAG bits
#define PACKET_FLAG_RELIABLE			(1<<0)	// packet contains subchannel stream data
#define PACKET_FLAG_COMPRESSED			(1<<1)	// packet is compressed
#define PACKET_FLAG_ENCRYPTED			(1<<2)  // packet is encrypted
#define PACKET_FLAG_SPLIT				(1<<3)  // packet is split
#define PACKET_FLAG_CHOKED				(1<<4)  // packet was choked by sender
#define PACKET_FLAG_CHALLENGE			(1<<5) // packet is a challenge
#define PACKET_FLAG_TABLES				(1<<10) //custom flag, request string tables

#define CONNECTION_PROBLEM_TIME		4.0f	// assume network problem after this time

#define BYTES2FRAGMENTS(i) ((i+FRAGMENT_SIZE-1)/FRAGMENT_SIZE)

#define FLIPBIT(v,b) if (v&b) v &= ~b; else v |= b;
// Flow control bytes per second limits
#define MAX_RATE		(1024*1024)				
#define MIN_RATE		1000
#define DEFAULT_RATE	10000

#define SIGNON_TIME_OUT				300.0f  // signon disconnect timeout

#define FRAGMENT_BITS		8//correct
#define FRAGMENT_SIZE		(1<<FRAGMENT_BITS)//correct
#define MAX_FILE_SIZE_BITS	26
#define MAX_FILE_SIZE		((1<<MAX_FILE_SIZE_BITS)-1)	// maximum transferable size is	64MB

// 0 == regular, 1 == file stream
#define MAX_STREAMS			2    

#define	FRAG_NORMAL_STREAM	0
#define FRAG_FILE_STREAM	1

#define TCP_CONNECT_TIMEOUT		4.0f
#define	PORT_ANY				-1
#define PORT_TRY_MAX			10
#define TCP_MAX_ACCEPTS			8

#define LOOPBACK_SOCKETS	2

#define STREAM_CMD_NONE		0	// waiting for next blob
#define STREAM_CMD_AUTH		1	// first command, send back challengenr
#define STREAM_CMD_DATA		2	// receiving a data blob
#define STREAM_CMD_FILE		3	// receiving a file blob
#define STREAM_CMD_ACKN		4	// acknowledged a recveived blob

// NETWORKING INFO

// This is the packet payload without any header bytes (which are attached for actual sending)
//#define	NET_MAX_PAYLOAD			80000	// largest message we can send in bytes
#define	NET_MAX_PAYLOAD			96000	// largest message we can send in bytes
#define NET_MAX_PAYLOAD_BITS	17		// 2^NET_MAX_PAYLOAD_BITS > NET_MAX_PAYLOAD
// This is just the client_t->netchan.datagram buffer size (shouldn't ever need to be huge)
#define NET_MAX_DATAGRAM_PAYLOAD 4000	// = maximum unreliable playload size

// UDP has 28 byte headers
#define UDP_HEADER_SIZE				(20+8)	// IP = 20, UDP = 8


#define MAX_ROUTABLE_PAYLOAD		1260	// Matches x360 size

#if (MAX_ROUTABLE_PAYLOAD & 3) != 0
#error Bit buffers must be a multiple of 4 bytes
#endif

#define MIN_ROUTABLE_PAYLOAD		16		// minimum playload size

#define NETMSG_TYPE_BITS	6	// must be 2^NETMSG_TYPE_BITS > SVC_LASTMSG

// This is the payload plus any header info (excluding UDP header)

#define HEADER_BYTES	( 8 + MAX_STREAMS * 9 )	// 2*4 bytes seqnr, 1 byte flags

// Pad this to next higher 16 byte boundary
// This is the largest packet that can come in/out over the wire, before processing the header
//  bytes will be stripped by the networking channel layer
#define	NET_MAX_MESSAGE	PAD_NUMBER( ( NET_MAX_PAYLOAD + HEADER_BYTES ), 16 )

#define MAX_STREAMS 2
#define FLOW_INCOMING 1

#define MAX_OSPATH 260
#define FileHandle_t FILE*

inline unsigned short BufferToShortChecksum(void *pvData, size_t nLength)
{
	CRC32_t crc = CRC32_ProcessSingleBuffer(pvData, nLength);

	unsigned short lowpart = (crc & 0xffff);
	unsigned short highpart = ((crc >> 16) & 0xffff);

	return (unsigned short)(lowpart ^ highpart);
}


// Increases and removes last 3 bits
#define UPPER_BOUND( x ) ( ( x + 7 ) & ~7 )
// Calculates amount of bits written in current byte
#define USED_BYTE_BITS( x ) ( x % 8 )
// Padding
#define ENCODE_PAD_BITS( x ) ( ( x << 5 ) & 0xff )

inline unsigned short CRC16_ProcessSingleBuffer(unsigned char *data, unsigned int size)
{
	int crc32 = CRC32_ProcessSingleBuffer(data, size);

	return (unsigned short)(crc32 ^ (crc32 >> 16));
}

inline bool NET_BufferToBufferDecompress(char *dest, unsigned int &destLen, char *source, unsigned int sourceLen)
{

	CLZSS s;
	if (s.IsCompressed((unsigned char *)source))
	{
		unsigned int uDecompressedLen = s.GetActualSize((unsigned char *)source);
		if (uDecompressedLen > destLen)
		{
			printf("NET_BufferToBufferDecompress with improperly sized dest buffer (%u in, %u needed)\n", destLen, uDecompressedLen);
			return false;
		}
		else
		{
			destLen = s.Uncompress((unsigned char *)source, (unsigned char *)dest);
		}
	}
	else
	{
		memcpy(dest, source, sourceLen);
		destLen = sourceLen;
	}

	return true;
}

inline bool NET_BufferToBufferCompress(char *dest, unsigned int *destLen, char *source, unsigned int sourceLen)
{

	memcpy(dest, source, sourceLen);
	CLZSS s;
	unsigned int uCompressedLen = 0;
	unsigned char *pbOut = s.Compress((unsigned char *)source, sourceLen, &uCompressedLen);
	if (pbOut && uCompressedLen > 0 && uCompressedLen <= *destLen)
	{
		memcpy(dest, pbOut, uCompressedLen);
		*destLen = uCompressedLen;
		free(pbOut);
	}
	else
	{
		if (pbOut)
		{
			free(pbOut);
		}
		memcpy(dest, source, sourceLen);
		*destLen = sourceLen;
		return false;
	}
	return true;
}

typedef struct dataFragments_s
{
	FileHandle_t	file;			// open file handle
	char			filename[MAX_OSPATH]; // filename
	char*			buffer;			// if NULL it's a file
	unsigned int	bytes;			// size in bytes
	unsigned int	bits;			// size in bits
	unsigned int	transferID;		// only for files
	bool			isCompressed;	// true if data is bzip compressed
	unsigned int	nUncompressedSize; // full size in bytes
	bool			asTCP;			// send as TCP stream
	int				numFragments;	// number of total fragments
	int				ackedFragments; // number of fragments send & acknowledged
	int				pendingFragments; // number of fragments send, but not acknowledged yet
	int fragmentOffsets[0xFFFF];
	int fragmentOffsets_num;

} dataFragments_t;

struct subChannel_s
{
	int				startFraggment[MAX_STREAMS];
	int				numFragments[MAX_STREAMS];
	int				sendSeqNr;
	int				state; // 0 = free, 1 = scheduled to send, 2 = send & waiting, 3 = dirty
	int				index; // index in m_SubChannels[]

	void Free()
	{
		state = SUBCHANNEL_FREE;
		sendSeqNr = -1;
		for (int i = 0; i < MAX_STREAMS; i++)
		{
			numFragments[i] = 0;
			startFraggment[i] = -1;
		}
	}
};

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

	int ProcessPacketHeader(int msgsize, bf_read &read);
	bool ReadSubChannelData(bf_read &buf, int stream);

	bool CheckReceivingList(int nList);
	void CheckWaitingList(int nList);
	void UncompressFragments(dataFragments_t *data);
	void RemoveHeadInWaitingList(int nList);
	bool NeedsFragments();
	int m_iForceNeedsFrags;

	int HandleSplitPacket(char*netrecbuffer, int &msgsize, bf_read&recvdata);

	inline void Initialize()
	{
		m_iServerCount = -1;
		m_iSignOnState = 2;
		m_iForceNeedsFrags = 0;
		m_bStreamContainsChallenge = false;
		m_ChallengeNr = 0;
		m_PacketDrop = 0;
		m_nInSequenceNr = 0;
		m_nOutSequenceNrAck = 0;
		m_nOutReliableState = 0;
		m_nInReliableState = 0;
		m_nOutSequenceNr = 1;

		m_ReceiveList[FRAG_NORMAL_STREAM].buffer = NULL;
		m_ReceiveList[FRAG_FILE_STREAM].buffer = NULL;
		for (int i = 0; i<MAX_SUBCHANNELS; i++)
		{
			m_SubChannels[i].index = i; // set index once
			m_SubChannels[i].Free();
		}

		m_WaitingList->clear();
		memset(m_ReceiveList, 0, sizeof(m_ReceiveList));
		memset(m_SubChannels, 0, sizeof(m_SubChannels));
		memset(m_WaitingList, 0, sizeof(m_WaitingList));


	}
};