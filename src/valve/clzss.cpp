//========= Copyright © 1996-2007, Valve Corporation, All rights reserved. ============//
//
//	LZSS Codec. Designed for fast cheap gametime encoding/decoding. Compression results
//	are	not aggresive as other alogrithms, but gets 2:1 on most arbitrary uncompressed data.
//
//=====================================================================================//

#include "clzss.h"

/*
#if _DEBUG
#define BITBUF_INLINE inline
#else
#define BITBUF_INLINE FORCEINLINE
#endif*/

#define BITBUF_INLINE FORCEINLINE
template <typename T>
inline T WordSwapC(T w)
{
	uint16 temp;

	temp = ((*((uint16 *)&w) & 0xff00) >> 8);
	temp |= ((*((uint16 *)&w) & 0x00ff) << 8);

	return *((T*)&temp);
}

template <typename T>
inline T DWordSwapC(T dw)
{
	uint32 temp;

	temp = *((uint32 *)&dw) >> 24;
	temp |= ((*((uint32 *)&dw) & 0x00FF0000) >> 8);
	temp |= ((*((uint32 *)&dw) & 0x0000FF00) << 8);
	temp |= ((*((uint32 *)&dw) & 0x000000FF) << 24);

	return *((T*)&temp);
}

//-------------------------------------
// Fast swaps
//-------------------------------------

#if defined( _X360 )

#define WordSwap  WordSwap360Intr
#define DWordSwap DWordSwap360Intr

template <typename T>
inline T WordSwap360Intr(T w)
{
	T output;
	__storeshortbytereverse(w, 0, &output);
	return output;
}

template <typename T>
inline T DWordSwap360Intr(T dw)
{
	T output;
	__storewordbytereverse(dw, 0, &output);
	return output;
}

#elif defined( _MSC_VER )

#define WordSwap  WordSwapAsm
#define DWordSwap DWordSwapAsm

#pragma warning(push)
#pragma warning (disable:4035) // no return value

template <typename T>
inline T WordSwapAsm(T w)
{
	__asm
	{
		mov ax, w
			xchg al, ah
	}
}

template <typename T>
inline T DWordSwapAsm(T dw)
{
	__asm
	{
		mov eax, dw
			bswap eax
	}
}

#pragma warning(pop)

#else

#define WordSwap  WordSwapC
#define DWordSwap DWordSwapC

#endif
#define BigShort( val )				WordSwap( val )
#define BigWord( val )				WordSwap( val )
#define BigLong( val )				DWordSwap( val )
#define BigDWord( val )				DWordSwap( val )
#define LittleShort( val )			( val )
#define LittleWord( val )			( val )
#define LittleLong( val )			( val )
#define LittleDWord( val )			( val )
#define LittleQWord( val )			( val )
#define SwapShort( val )			BigShort( val )
#define SwapWord( val )				BigWord( val )
#define SwapLong( val )				BigLong( val )
#define SwapDWord( val )			BigDWord( val )
#define LittleFloat( pOut, pIn )	( *pOut = *pIn )

#define Assert(val) if(val) { }

inline unsigned long LoadLittleDWord(const unsigned long *base, unsigned int dwordIndex)
{
	return LittleDWord(base[dwordIndex]);
}

inline void StoreLittleDWord(unsigned long *base, unsigned int dwordIndex, unsigned long dword)
{
	base[dwordIndex] = LittleDWord(dword);
}


#define LZSS_LOOKSHIFT		4
#define LZSS_LOOKAHEAD		( 1 << LZSS_LOOKSHIFT )
#include <stdio.h>
//-----------------------------------------------------------------------------
// Returns true if buffer is compressed.
//-----------------------------------------------------------------------------
bool CLZSS::IsCompressed( unsigned char *pInput )
{
	lzss_header_t *pHeader = (lzss_header_t *)pInput;

	//printf("LOL: %x __ %x\n", pHeader->id, LZSS_ID);
	if ( pHeader && pHeader->id == LZSS_ID )
	{
		return true;
	}

	// unrecognized
	return false;
}

//-----------------------------------------------------------------------------
// Returns uncompressed size of compressed input buffer. Used for allocating output
// buffer for decompression. Returns 0 if input buffer is not compressed.
//-----------------------------------------------------------------------------
unsigned int CLZSS::GetActualSize( unsigned char *pInput )
{
	lzss_header_t *pHeader = (lzss_header_t *)pInput;
	if ( pHeader && pHeader->id == LZSS_ID )
	{
		return LittleLong( pHeader->actualSize );
	}

	// unrecognized
	return 0;
}

void CLZSS::BuildHash( unsigned char *pData )
{
	lzss_list_t *pList;
	lzss_node_t *pTarget;

	int targetindex = (unsigned int)pData & ( m_nWindowSize - 1 );
	pTarget = &m_pHashTarget[targetindex];
	if ( pTarget->pData )
	{
		pList = &m_pHashTable[*pTarget->pData];
		if ( pTarget->pPrev )
		{
			pList->pEnd = pTarget->pPrev;
			pTarget->pPrev->pNext = 0;
		}
		else
		{
			pList->pEnd = 0;
			pList->pStart = 0;
		}
	}

	pList = &m_pHashTable[*pData];
	pTarget->pData = pData;
	pTarget->pPrev = 0;
	pTarget->pNext = pList->pStart;
	if ( pList->pStart )
	{
		pList->pStart->pPrev = pTarget;
	}
	else
	{
		pList->pEnd = pTarget;
	}
	pList->pStart = pTarget;
}

#include <string.h>
#include <stdlib.h>
#define stackalloc malloc

unsigned char *CLZSS::CompressNoAlloc( unsigned char *pInput, int inputLength, unsigned char *pOutputBuf, unsigned int *pOutputSize )
{
	if ( inputLength <= sizeof( lzss_header_t ) + 8 )
	{
		return 0;
	}

	// create the compression work buffers, small enough (~64K) for stack
	m_pHashTable = (lzss_list_t *)stackalloc( 256 * sizeof( lzss_list_t ) );
	memset( m_pHashTable, 0, 256 * sizeof( lzss_list_t ) );
	m_pHashTarget = (lzss_node_t *)stackalloc( m_nWindowSize * sizeof( lzss_node_t ) );
	memset( m_pHashTarget, 0, m_nWindowSize * sizeof( lzss_node_t ) );

	// allocate the output buffer, compressed buffer is expected to be less, caller will free
	unsigned char *pStart = pOutputBuf;
	// prevent compression failure (inflation), leave enough to allow dribble eof bytes
	unsigned char *pEnd = pStart + inputLength - sizeof ( lzss_header_t ) - 8;

	// set the header
	lzss_header_t *pHeader = (lzss_header_t *)pStart;
	pHeader->id = LZSS_ID;
	pHeader->actualSize = LittleLong( inputLength );

	unsigned char *pOutput = pStart + sizeof (lzss_header_t);
	unsigned char *pLookAhead = pInput; 
	unsigned char *pWindow = pInput;
	unsigned char *pEncodedPosition = NULL;
	unsigned char *pCmdByte = NULL;
	int putCmdByte = 0;

	while ( inputLength > 0 )
	{
		pWindow = pLookAhead - m_nWindowSize;
		if ( pWindow < pInput )
		{
			pWindow = pInput;
		}

		if ( !putCmdByte )
		{
			pCmdByte = pOutput++;
			*pCmdByte = 0;
		}
		putCmdByte = ( putCmdByte + 1 ) & 0x07;

		int encodedLength = 0;
		int lookAheadLength = inputLength < LZSS_LOOKAHEAD ? inputLength : LZSS_LOOKAHEAD;

		lzss_node_t *pHash = m_pHashTable[pLookAhead[0]].pStart;
		while ( pHash )
		{
			int matchLength = 0;
			int length = lookAheadLength;
			while ( length-- && pHash->pData[matchLength] == pLookAhead[matchLength] )
			{
				matchLength++;
			}
			if ( matchLength > encodedLength )
			{
				encodedLength = matchLength;
				pEncodedPosition = pHash->pData;
			}
			if ( matchLength == lookAheadLength )
			{
				break;
			}
			pHash = pHash->pNext;
		}

		if ( encodedLength >= 3 )
		{
			*pCmdByte = ( *pCmdByte >> 1 ) | 0x80;
			*pOutput++ = ( ( pLookAhead-pEncodedPosition-1 ) >> LZSS_LOOKSHIFT );
			*pOutput++ = ( ( pLookAhead-pEncodedPosition-1 ) << LZSS_LOOKSHIFT ) | ( encodedLength-1 );
		} 
		else 
		{ 
			encodedLength = 1;
			*pCmdByte = ( *pCmdByte >> 1 );
			*pOutput++ = *pLookAhead;
		}

		for ( int i=0; i<encodedLength; i++ )
		{
			BuildHash( pLookAhead++ );
		}

		inputLength -= encodedLength;

		if ( pOutput >= pEnd )
		{
			// compression is worse, abandon
			return NULL;
		}
	}

	if ( inputLength != 0 )
	{
		// unexpected failure
		Assert( 0 );
		return NULL;
	}

	if ( !putCmdByte )
	{
		pCmdByte = pOutput++;
		*pCmdByte = 0x01;
	}
	else
	{
		*pCmdByte = ( ( *pCmdByte >> 1 ) | 0x80 ) >> ( 7 - putCmdByte );
	}

	*pOutput++ = 0;
	*pOutput++ = 0;

	if ( pOutputSize )
	{
		*pOutputSize = pOutput - pStart;
	}

	return pStart;
}

//-----------------------------------------------------------------------------
// Compress an input buffer. Caller must free output compressed buffer.
// Returns NULL if compression failed (i.e. compression yielded worse results)
//-----------------------------------------------------------------------------
unsigned char* CLZSS::Compress( unsigned char *pInput, int inputLength, unsigned int *pOutputSize )
{
	unsigned char *pStart = (unsigned char *)malloc( inputLength );
	unsigned char *pFinal = CompressNoAlloc( pInput, inputLength, pStart, pOutputSize );
	if ( !pFinal )
	{
		free( pStart );
		return NULL;
	}

	return pStart;
}

//-----------------------------------------------------------------------------
// Uncompress a buffer, Returns the uncompressed size. Caller must provide an
// adequate sized output buffer or memory corruption will occur.
//-----------------------------------------------------------------------------
unsigned int CLZSS::Uncompress( unsigned char *pInput, unsigned char *pOutput )
{
	unsigned int totalBytes = 0;
	int cmdByte = 0;
	int getCmdByte = 0;

	unsigned int actualSize = GetActualSize( pInput );
	if ( !actualSize )
	{
		// unrecognized
		return 0;
	}

	pInput += sizeof( lzss_header_t );

	for ( ;; )
	{
		if ( !getCmdByte ) 
		{
			cmdByte = *pInput++;
		}
		getCmdByte = ( getCmdByte + 1 ) & 0x07;

		if ( cmdByte & 0x01 )
		{
			int position = *pInput++ << LZSS_LOOKSHIFT;
			position |= ( *pInput >> LZSS_LOOKSHIFT );
			int count = ( *pInput++ & 0x0F ) + 1;
			if ( count == 1 ) 
			{
				break;
			}
			unsigned char *pSource = pOutput - position - 1;
			for ( int i=0; i<count; i++ )
			{
				*pOutput++ = *pSource++;
			}
			totalBytes += count;
		} 
		else 
		{
			*pOutput++ = *pInput++;
			totalBytes++;
		}
		cmdByte = cmdByte >> 1;
	}

	if ( totalBytes != actualSize )
	{
		return 0;
	}

	return totalBytes;
}


