//========= Copyright Valve Corporation, All rights reserved. ============//
//
// $Header: $
// $NoKeywords: $
//
// Serialization buffer
//===========================================================================//

#pragma warning (disable : 4514)

#include "utlbuffer.h"
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdlib.h>
#include <limits.h>
#include <Windows.h>

//-----------------------------------------------------------------------------
// Character conversions for C strings
//-----------------------------------------------------------------------------
class CUtlCStringConversion : public CUtlCharConversion
{
public:
	CUtlCStringConversion( char nEscapeChar, const char *pDelimiter, int nCount, ConversionArray_t *pArray );

	// Finds a conversion for the passed-in string, returns length
	virtual char FindConversion( const char *pString, int *pLength );

private:
	char m_pConversion[256];
};


//-----------------------------------------------------------------------------
// Character conversions for no-escape sequence strings
//-----------------------------------------------------------------------------
class CUtlNoEscConversion : public CUtlCharConversion
{
public:
	CUtlNoEscConversion( char nEscapeChar, const char *pDelimiter, int nCount, ConversionArray_t *pArray ) :
		CUtlCharConversion( nEscapeChar, pDelimiter, nCount, pArray ) {}

	// Finds a conversion for the passed-in string, returns length
	virtual char FindConversion( const char *pString, int *pLength ) { *pLength = 0; return 0; }
};


//-----------------------------------------------------------------------------
// List of character conversions
//-----------------------------------------------------------------------------
BEGIN_CUSTOM_CHAR_CONVERSION( CUtlCStringConversion, s_StringCharConversion, "\"", '\\' )
	{ '\n', "n" },
	{ '\t', "t" },
	{ '\v', "v" },
	{ '\b', "b" },
	{ '\r', "r" },
	{ '\f', "f" },
	{ '\a', "a" },
	{ '\\', "\\" },
	{ '\?', "\?" },
	{ '\'', "\'" },
	{ '\"', "\"" },
END_CUSTOM_CHAR_CONVERSION( CUtlCStringConversion, s_StringCharConversion, "\"", '\\' )

CUtlCharConversion *GetCStringCharConversion()
{
	return &s_StringCharConversion;
}

BEGIN_CUSTOM_CHAR_CONVERSION( CUtlNoEscConversion, s_NoEscConversion, "\"", 0x7F )
	{ 0x7F, "" },
END_CUSTOM_CHAR_CONVERSION( CUtlNoEscConversion, s_NoEscConversion, "\"", 0x7F )

CUtlCharConversion *GetNoEscCharConversion()
{
	return &s_NoEscConversion;
}


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CUtlCStringConversion::CUtlCStringConversion( char nEscapeChar, const char *pDelimiter, int nCount, ConversionArray_t *pArray ) : 
	CUtlCharConversion( nEscapeChar, pDelimiter, nCount, pArray )
{
	memset( m_pConversion, 0x0, sizeof(m_pConversion) );
	for ( int i = 0; i < nCount; ++i )
	{
		m_pConversion[ (unsigned char) pArray[i].m_pReplacementString[0] ] = pArray[i].m_nActualChar;
	}
}

// Finds a conversion for the passed-in string, returns length
char CUtlCStringConversion::FindConversion( const char *pString, int *pLength )
{
	char c = m_pConversion[ (unsigned char) pString[0] ];
	*pLength = (c != '\0') ? 1 : 0;
	return c;
}



//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CUtlCharConversion::CUtlCharConversion( char nEscapeChar, const char *pDelimiter, int nCount, ConversionArray_t *pArray )
{
	m_nEscapeChar = nEscapeChar;
	m_pDelimiter = pDelimiter;
	m_nCount = nCount;
	m_nDelimiterLength = strlen( pDelimiter );
	m_nMaxConversionLength = 0;

	memset( m_pReplacements, 0, sizeof(m_pReplacements) );

	for ( int i = 0; i < nCount; ++i )
	{
		m_pList[i] = pArray[i].m_nActualChar;
		ConversionInfo_t &info = m_pReplacements[ (unsigned char) m_pList[i] ];

		info.m_pReplacementString = pArray[i].m_pReplacementString;
		info.m_nLength = strlen( info.m_pReplacementString );
		if ( info.m_nLength > m_nMaxConversionLength )
		{
			m_nMaxConversionLength = info.m_nLength;
		}
	}
}


//-----------------------------------------------------------------------------
// Escape character + delimiter
//-----------------------------------------------------------------------------
char CUtlCharConversion::GetEscapeChar() const
{
	return m_nEscapeChar;
}

const char *CUtlCharConversion::GetDelimiter() const
{
	return m_pDelimiter;
}

int CUtlCharConversion::GetDelimiterLength() const
{
	return m_nDelimiterLength;
}


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
const char *CUtlCharConversion::GetConversionString( char c ) const
{
	return m_pReplacements[ (unsigned char) c ].m_pReplacementString;
}

int CUtlCharConversion::GetConversionLength( char c ) const
{
	return m_pReplacements[ (unsigned char) c ].m_nLength;
}

int CUtlCharConversion::MaxConversionLength() const
{
	return m_nMaxConversionLength;
}


//-----------------------------------------------------------------------------
// Finds a conversion for the passed-in string, returns length
//-----------------------------------------------------------------------------
char CUtlCharConversion::FindConversion( const char *pString, int *pLength )
{
	for ( int i = 0; i < m_nCount; ++i )
	{
		if ( !strcmp( pString, m_pReplacements[ (unsigned char) m_pList[i] ].m_pReplacementString ) )
		{
			*pLength = m_pReplacements[ (unsigned char) m_pList[i] ].m_nLength;
			return m_pList[i];
		}
	}

	*pLength = 0;
	return '\0';
}



CUtlBuffer::CUtlBuffer(  int nSize, int nFlags )
{

	m_Get = 0;
	m_Put = 0;
	m_nTab = 0;
	m_nOffset = 0;
	m_Flags = nFlags;
	if ( IsReadOnly() )
	{
		m_nMaxPut = nSize;
	}
	else
	{
		m_nMaxPut = -1;
		AddNullTermination();
	}

	m_Memory = new unsigned char[0xFFFFF];
	memset(m_Memory, 0, 0xFFFFF);
	SetOverflowFuncs( &CUtlBuffer::GetOverflow, &CUtlBuffer::PutOverflow );
}


//-----------------------------------------------------------------------------
// Modifies the buffer to be binary or text; Blows away the buffer and the CONTAINS_CRLF value. 
//-----------------------------------------------------------------------------
void CUtlBuffer::SetBufferType( bool bIsText, bool bContainsCRLF )
{
#ifdef _DEBUG
	// If the buffer is empty, there is no opportunity for this stuff to fail
	if ( TellMaxPut() != 0 )
	{
		if ( IsText() )
		{
			if ( bIsText )
			{
				Assert( ContainsCRLF() == bContainsCRLF );
			}
			else
			{
				Assert( ContainsCRLF() );
			}
		}
		else
		{
			if ( bIsText )
			{
				Assert( bContainsCRLF );
			}
		}
	}
#endif

	if ( bIsText )
	{
		m_Flags |= TEXT_BUFFER;
	}
	else
	{
		m_Flags &= ~TEXT_BUFFER;
	}
	if ( bContainsCRLF )
	{
		m_Flags |= CONTAINS_CRLF;
	}
	else
	{
		m_Flags &= ~CONTAINS_CRLF;
	}
}





	
//-----------------------------------------------------------------------------
// Serialization
//-----------------------------------------------------------------------------
void CUtlBuffer::Put( const void *pMem, int size )
{
	printf("PUT: %i\n", size);
	int Index = m_Put - m_nOffset;

	memcpy( m_Memory + Index , pMem, size );
	m_Put += size;

	AddNullTermination();
}


//-----------------------------------------------------------------------------
// Writes a null-terminated string
//-----------------------------------------------------------------------------
void CUtlBuffer::PutString( const char* pString )
{

	if ( pString )
	{
		// Not text? append a null at the end.
		size_t nLen = strlen( pString );
		for (unsigned int i = 0; i < nLen;i++)
			PutTypeBin<char>(pString[i]);

		PutTypeBin<char>(0);

		return;
	}
	else
	{
			PutTypeBin<char>( 0 );
	}
	
}


//-----------------------------------------------------------------------------
// This version of PutString converts \ to \\ and " to \", etc.
// It also places " at the beginning and end of the string
//-----------------------------------------------------------------------------
inline void CUtlBuffer::PutDelimitedCharInternal( CUtlCharConversion *pConv, char c )
{
	int l = pConv->GetConversionLength( c );
	if ( l == 0 )
	{
		PutChar( c );
	}
	else
	{
		PutChar( pConv->GetEscapeChar() );
		Put( pConv->GetConversionString( c ), l );
	}
}

void CUtlBuffer::PutDelimitedChar( CUtlCharConversion *pConv, char c )
{
	if ( !IsText() || !pConv )
	{
		PutChar( c );
		return;
	}

	PutDelimitedCharInternal( pConv, c );
}

void CUtlBuffer::PutDelimitedString( CUtlCharConversion *pConv, const char *pString )
{
	if ( !IsText() || !pConv )
	{
		PutString( pString );
		return;
	}

	if ( WasLastCharacterCR() )
	{
		PutTabs();
	}
	Put( pConv->GetDelimiter(), pConv->GetDelimiterLength() );

	int nLen = pString ? strlen( pString ) : 0;
	for ( int i = 0; i < nLen; ++i )
	{
		PutDelimitedCharInternal( pConv, pString[i] );
	}

	if ( WasLastCharacterCR() )
	{
		PutTabs();
	}
	Put( pConv->GetDelimiter(), pConv->GetDelimiterLength() );
}



//-----------------------------------------------------------------------------
// Calls the overflow functions
//-----------------------------------------------------------------------------
void CUtlBuffer::SetOverflowFuncs( UtlBufferOverflowFunc_t getFunc, UtlBufferOverflowFunc_t putFunc )
{
	m_GetOverflowFunc = getFunc;
	m_PutOverflowFunc = putFunc;
}

	
//-----------------------------------------------------------------------------
// Calls the overflow functions
//-----------------------------------------------------------------------------
bool CUtlBuffer::OnPutOverflow( int nSize )
{
	return (this->*m_PutOverflowFunc)( nSize );
}

bool CUtlBuffer::OnGetOverflow( int nSize )
{
	return (this->*m_GetOverflowFunc)( nSize );
}

	
//-----------------------------------------------------------------------------
// Checks if a put is ok
//-----------------------------------------------------------------------------
bool CUtlBuffer::PutOverflow( int nSize )
{

	return true;
}

bool CUtlBuffer::GetOverflow( int nSize )
{
	return false;
}
	

//-----------------------------------------------------------------------------
// Checks if a put is ok
//-----------------------------------------------------------------------------
bool CUtlBuffer::CheckPut( int nSize )
{
	if ( ( m_Error & PUT_OVERFLOW ) || IsReadOnly() )
		return false;

	if ( ( m_Put < m_nOffset ) )
	{
		if ( !OnPutOverflow( nSize ) )
		{
			m_Error |= PUT_OVERFLOW;
			return false;
		}
	}
	return true;
}

void CUtlBuffer::SeekPut( SeekType_t type, int offset )	
{
	int nNextPut = m_Put;
	switch( type )
	{
	case SEEK_HEAD:						 
		nNextPut = offset; 
		break;

	case SEEK_CURRENT:
		nNextPut += offset;
		break;

	case SEEK_TAIL:
		nNextPut = m_nMaxPut - offset;
		break;
	}

	// Force a write of the data
	// FIXME: We could make this more optimal potentially by writing out
	// the entire buffer if you seek outside the current range

	// NOTE: This call will write and will also seek the file to nNextPut.
	OnPutOverflow( -nNextPut-1 );
	m_Put = nNextPut;

	AddNullTermination();
}


void CUtlBuffer::ActivateByteSwapping( bool bActivate )
{

}

void CUtlBuffer::SetBigEndian( bool bigEndian )
{

}

bool CUtlBuffer::IsBigEndian( void )
{
	return false;
}


//-----------------------------------------------------------------------------
// null terminate the buffer
//-----------------------------------------------------------------------------
void CUtlBuffer::AddNullTermination( void )
{
	if ( m_Put > m_nMaxPut )
	{
		if ( !IsReadOnly() && ((m_Error & PUT_OVERFLOW) == 0)  )
		{
			// Add null termination value
			if ( CheckPut( 1 ) )
			{
				int Index = m_Put - m_nOffset;
				if( Index >= 0 )
				{
					m_Memory[ Index ] = 0;
				}
			}
			else
			{
				// Restore the overflow state, it was valid before...
				m_Error &= ~PUT_OVERFLOW;
			}
		}
		m_nMaxPut = m_Put;
	}		
}
