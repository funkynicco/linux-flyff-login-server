/*
 * File:   Utilities.inl
 * Author: nicco
 *
 * Created on June 12, 2014, 12:14 PM
 */

#pragma once

/*
 * Resolves a domain name into an IP address.
 * Can also be used on IP addresses directly.
 */
inline bool ResolveAddress( sockaddr_in* addr, const char* address )
{
    struct hostent* lphost = gethostbyname( address );
    if( !lphost )
        return false;

    memcpy( &addr->sin_addr.s_addr, lphost->h_addr, lphost->h_length );
    return true;
}

inline bool GetIPAddress( sockaddr_in* addr, char* output, size_t outputLen )
{
    if( outputLen < 16 ) // 255.255.255.255\0
        return false;

    sprintf(
        output,
        "%d.%d.%d.%d",
        (unsigned char)addr->sin_addr.s_addr,
        (unsigned char)( addr->sin_addr.s_addr >> 8 ),
        (unsigned char)( addr->sin_addr.s_addr >> 16 ),
        (unsigned char)( addr->sin_addr.s_addr >> 24 ) );

    return true;
}

/*
 * Sets a socket in blocking or non blocking mode.
 */
inline bool SetSocketBlocking( SOCKET s, bool isBlocking )
{
#ifdef _WIN32
    unsigned long mode = isBlocking ? 0 : 1;
    int n;
    if( ( n = ioctlsocket( s, FIONBIO, &mode ) ) != 0 )
    {
        DBGMSG_F( "ioctlsocket returned %d", n );
        return false;
    }
#else // _WIN32
    int flags = fcntl( s, F_GETFL, 0 );
    if( flags == -1 )
    {
        DBGMSG( "fcntl F_GETFL returned -1" );
        return false;
    }

    if( isBlocking )
        flags ^= O_NONBLOCK;
    else
        flags |= O_NONBLOCK;

    if( fcntl( s, F_SETFL, flags ) == -1 )
    {
        DBGMSG( "fcntl F_SETFL returned -1" );
        return false;
    }
#endif // _WIN32

    return true;
}

/*
 * Determines if a character is valid to be printed out in console.
 */
inline bool IsValidChar( char c )
{
    if( isalpha( c ) ||
        isdigit( c ) )
        return true;

    switch( c )
    {
    case ' ':
    case '.':
    case '/':
    case '\\':
    case '"':
    case '\'':
    case ';':
    case ':':
    case ',':
    case '*':
    case '?':
    case '#':
    case '!':
    case '@':
    case '\r':
    case '\n':
    case '-':
    case '+':
    case '(':
    case ')':
    case '{':
    case '}':
    case '=':
        return true;
    }

    return false;
}