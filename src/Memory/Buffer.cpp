/*
 * File:   Buffer.cpp
 * Author: nicco
 *
 * Created on June 13, 2014, 09:06 AM
 */

#include "stdafx.h"
#include "Buffer.h"

#define MAX_FILESIZE 67108864 //64mb //1024

CAr::CAr( void* lpBuf, size_t nBufSize )
{
    Reset( lpBuf, nBufSize );
}

CAr::~CAr()
{
    if( IsStoring() &&
        m_nBufSize > BUFFER_SIZE )
        free( m_lpBufStart );
}

void CAr::Read( void* lpBuf, size_t nSize )
{
    if( m_lpBufCur + nSize > m_lpBufEnd )
        return;

    memcpy( lpBuf, m_lpBufCur, nSize );
    m_lpBufCur += nSize;
}

void CAr::Write( const void* lpBuf, size_t nSize )
{
    CheckBuf( nSize );

    memcpy( m_lpBufCur, lpBuf, nSize );
    m_lpBufCur += nSize;
    m_lpBufMax = max( m_lpBufMax, m_lpBufCur );
}

void CAr::ReadString( char* szString, size_t nMaxLen )
{
    int len = 0;
    Read( &len, sizeof( int ) );
    if( len > 0 && (size_t)len < nMaxLen )
    {
        Read( szString, len );
        *( szString + len ) = 0;
    }
}

void CAr::WriteString( const char* szString )
{
    int len = (int)strlen( szString );
    Write( &len, sizeof( int ) );
    Write( szString, len );
}

void CAr::CheckBuf( size_t nSize )
{
    size_t offset = GetOffset();
    size_t length = GetLength();
    size_t newsize = m_nBufSize;

    while( offset + nSize > newsize )
        newsize *= 2;

    if( newsize == m_nBufSize ) // No change needed
        return;

    if( m_nBufSize > BUFFER_SIZE )
    {
        m_lpBufStart = (unsigned char*)realloc( m_lpBufStart, newsize );
    }
    else
    {
        unsigned char* lpBuf = (unsigned char*)malloc( newsize );
        memcpy( lpBuf, m_lpBufStart, m_nBufSize );
        m_lpBufStart = lpBuf;
    }

    m_nBufSize = newsize;
    m_lpBufCur = m_lpBufStart + offset;
    m_lpBufEnd = m_lpBufStart + m_nBufSize;
    m_lpBufMax = m_lpBufStart + length;
}

void CAr::Remove( size_t nLength )
{
    if( nLength <= GetLength() )
    {
        memcpy( m_lpBufStart, m_lpBufStart + nLength, GetLength() - nLength );
        m_lpBufCur -= nLength;
        m_lpBufMax -= nLength;
        if( m_lpBufCur < m_lpBufStart )
            m_lpBufCur = m_lpBufStart;
    }
}

void CAr::Insert( size_t nIndex, void* lpBuf, size_t nLength )
{
    if( nIndex > GetLength() )
        return;

    CheckBuf( GetLength() + nLength );
    //memcpy( m_lpBufStart + nIndex + nLength, m_lpBufStart + nIndex, nLength );
    /*
    insert: ac cd 99 de
    ----------------------------------------------
    10 1a 3c 2d aa 9c 8c
    ^ insert point (pos 3b, remaining 4b)
    -----------> 2d aa 9c 8c
    10 1a 3c             2d aa 9c 8c
    10 1a 3c ac cd 99 de 2d aa 9c 8c
    */

    size_t remain = GetLength() - nIndex;
    memcpy( m_lpBufStart + nIndex + nLength, m_lpBufStart + nIndex, remain );
    memcpy( m_lpBufStart + nIndex, lpBuf, nLength );
    m_lpBufCur += nLength;
    m_lpBufMax += nLength;
}

unsigned char* CAr::Reserve( size_t nSize )
{
    CheckBuf( nSize );

    unsigned char* lpBufCur = m_lpBufCur;

    m_lpBufCur += nSize;
    m_lpBufMax = max( m_lpBufMax, m_lpBufCur );

    return lpBufCur;
}

void CAr::Reset( void* lpBuf, size_t nBufSize )
{
    if( lpBuf )
    {
        m_bIsStoring = false;
        m_lpBufStart = (unsigned char*)lpBuf;
        m_nBufSize = nBufSize;
        m_lpBufMax = (unsigned char*)lpBuf + nBufSize;
    }
    else
    {
        m_bIsStoring = true;
        m_lpBufStart = (unsigned char*)m_lpBuf;
        m_nBufSize = BUFFER_SIZE;
        m_lpBufMax = m_lpBufStart;
    }

    m_lpBufEnd = m_lpBufStart + m_nBufSize;
    m_lpBufCur = m_lpBufStart;
}

void CAr::Flush()
{
    m_lpBufCur = m_lpBufStart;
    m_lpBufMax = m_lpBufStart;
}