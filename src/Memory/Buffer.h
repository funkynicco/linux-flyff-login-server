/*
 * File:   Buffer.h
 * Author: nicco
 *
 * Created on June 13, 2014, 09:06 AM
 */

#pragma once

#define BUFFER_SIZE 16384

class CAr
{
public:
    CAr( void* lpBuf = NULL, size_t nBufSize = 0 );
    virtual ~CAr();

    void Read( void* lpBuf, size_t nSize );
    void Write( const void* lpBuf, size_t nSize );

    void ReadString( char* szString, size_t nMaxLen );
    void WriteString( const char* szString );

    void CheckBuf( size_t nSize );
    void Remove( size_t nLength );
    void Insert( size_t nIndex, void* lpBuf, size_t nLength );

    /* Reserves a range of bytes that can be written to later on. */
    unsigned char* Reserve( size_t nSize );
    void Flush();
    void Reset( void* lpBuf = NULL, size_t nBufSize = 0 );

    inline bool IsLoading() { return !m_bIsStoring; }
    inline bool IsStoring() { return m_bIsStoring; }
    inline size_t GetOffset() { return m_lpBufCur - m_lpBufStart; }
    inline unsigned char* GetBuffer() { return m_lpBufStart; }
    inline size_t GetBufferSize() { return m_nBufSize; }
    inline size_t GetLength() { return m_lpBufMax - m_lpBufStart; }
    inline size_t GetRemain() { return m_lpBufMax - m_lpBufCur; }

    inline void SetIsStoring( bool bIsStoring ) { m_bIsStoring = bIsStoring; }

    inline bool SetOffset( size_t nOffset )
    {
        if( m_lpBufStart + nOffset > m_lpBufEnd )
            return false;

        m_lpBufCur = m_lpBufStart + nOffset;
        return true;
    }

    template <typename T>
    inline CAr& operator <<( const T& obj )
    {
        Write( &obj, sizeof( T ) );
        return *this;
    }

    template <typename T>
    inline CAr& operator >>( T& obj )
    {
        Read( &obj, sizeof( T ) );
        return *this;
    }

private:
    bool m_bIsStoring;
    size_t m_nBufSize;
    unsigned char* m_lpBufCur;		// Current pointer
    unsigned char* m_lpBufEnd;		// End of buffered memory
    unsigned char* m_lpBufStart;	// Begin
    unsigned char* m_lpBufMax;		// Maximum valid data in current buffer (Length, as opposed to the end)
    unsigned char m_lpBuf[ BUFFER_SIZE ];
};