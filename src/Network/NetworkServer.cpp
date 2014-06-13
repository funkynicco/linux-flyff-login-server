#include "stdafx.h"
#include "NetworkServer.h"

#define MAX_EVENTS 32768

CNetworkServer::CNetworkServer() :
m_sock( INVALID_SOCKET )
#ifdef __linux
,m_efd( -1 )
,m_events( NULL )
#endif // __linux
{
#ifdef _WIN32
    m_tv.tv_sec = 0;
    m_tv.tv_usec = 1;
#endif // _WIN32
}

CNetworkServer::~CNetworkServer()
{
    Close();
}

bool CNetworkServer::Start( uint16_t port )
{
    if( m_sock != INVALID_SOCKET )
    {
        DBGMSG( "m_sock is not INVALID_SOCKET" );
        return false;
    }

#ifdef _WIN32
    int protocol = IPPROTO_TCP;
#else // _WIN32
    int protocol = 0;
#endif // _WIN32
    if( ( m_sock = socket( AF_INET, SOCK_STREAM, protocol ) ) == INVALID_SOCKET )
    {
        DBGMSG( "socket() returned INVALID_SOCKET" );
        return false;
    }

#ifdef __linux
    int val = 1;
    if( setsockopt( m_sock, SOL_SOCKET, SO_REUSEADDR, &val, sizeof ( int ) ) == -1 )
        DBGMSG_F( "Failed to set SO_REUSEADDR on socket %d (error: %d)", m_sock, ERR_NR );
#endif // __linux

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons( port );

    int a = 0, b = 0;

    if( ( a = bind( m_sock, ( struct sockaddr* ) &addr, sizeof ( addr ) ) ) == SOCKET_ERROR ||
        ( b = listen( m_sock, SOMAXCONN ) ) == SOCKET_ERROR )
    {
        DBGMSG_F( "bind(%d) or listen(%d) failed, errno: %d", a, b, ERR_NR );

        Close();
        return false;
    }

#ifdef __linux
    if( !SetSocketBlocking( m_sock, false ) )
    {
        DBGMSG( "Failed to set socket as non-blocking" );
        Close();
        return false;
    }

    if( ( m_efd = epoll_create1( 0 ) ) == -1 )
    {
        DBGMSG( "epoll_create1 returned -1" );
        Close();
        return false;
    }

    m_event.data.fd = m_sock;
    m_event.events = EPOLLIN | EPOLLET;
    if( epoll_ctl( m_efd, EPOLL_CTL_ADD, m_sock, &m_event ) == -1 )
    {
        DBGMSG( "epoll_ctl returned -1" );
        Close();
        return false;
    }

    m_events = ( struct epoll_event* ) malloc( sizeof ( struct epoll_event ) * MAX_EVENTS );
    //#ifdef _DEBUG
    memset( m_events, 0, sizeof ( struct epoll_event ) * MAX_EVENTS );
    //#endif // _DEBUG
#endif // __linux

    return true;
}

void CNetworkServer::Close()
{
    if( m_sock == INVALID_SOCKET )
    {
        DBGMSG( "m_sock is already INVALID_SOCKET" );
        return;
    }

    map<SOCKET, ServerClient*>::iterator it;
    while( ( it = m_clients.begin() ) != m_clients.end() )
    {
        ServerClient* client = it->second;
        m_clients.erase( it );

        if( shutdown( client->Socket, SHUT_RDWR ) == -1 )
            DBGMSG_F( "shutdown failed on client %d with error: %d", client->Socket, ERR_NR );
        closesocket( client->Socket );
        OnClientDisconnected( client );
        FreeClient( client );
    }

    if( shutdown( m_sock, SHUT_RD ) == -1 )
        DBGMSG_F( "shutdown failed on listening socket with error: %d", ERR_NR );
    closesocket( m_sock );
    m_sock = INVALID_SOCKET;

#ifdef __linux
    if( m_events )
    {
        free( m_events );
        m_events = NULL;
    }
#endif // __linux

    DBGMSG( "Close completed" );
}

void CNetworkServer::Process( time_t tm )
{
    if( m_sock == INVALID_SOCKET )
        return;

#ifdef __linux
    int n = epoll_wait( m_efd, m_events, MAX_EVENTS, 1 );
    for( int i = 0; i < n; ++i )
    {
        if( ( m_events[ i ].events & EPOLLERR ) ||
            ( m_events[ i ].events & EPOLLHUP ) ||
            ( !( m_events[ i ].events & EPOLLIN ) ) )
        {
            /* An error has occured on this fd, or the socket is not
             * ready for reading (why were we notified then?)
             */

            closesocket( m_events[ i ].data.fd );

            map<SOCKET, ServerClient*>::iterator it = m_clients.find( m_events[ i ].data.fd );
            if( it != m_clients.end() )
            {
                ServerClient* client = it->second;
                m_clients.erase( it );
                OnClientDisconnected( client );
                FreeClient( client );
                continue;
            }
        }
        else if( m_events[ i ].data.fd == m_sock )
        {
            // Process incoming connection
            struct sockaddr_in addr;
            while( 1 )
            {
                socklen_t addrlen = sizeof ( addr );
                SOCKET c = accept( m_sock, ( struct sockaddr* ) &addr, &addrlen );
                if( c != INVALID_SOCKET )
                {
                    ServerClient* client = AllocateClient( c );
                    GetIPAddress( &addr, client->szIP, sizeof ( client->szIP ) );
                    m_clients.insert( pair<SOCKET, ServerClient*>( c, client ) );
                    OnClientConnected( client );

                    if( !SetSocketBlocking( c, false ) )
                        DBGMSG_F( "Failed to set non-blocking on client %d", c );

                    m_event.data.fd = c;
                    m_event.events = EPOLLIN | EPOLLET;
                    if( epoll_ctl( m_efd, EPOLL_CTL_ADD, c, &m_event ) == -1 )
                        DBGMSG_F( "epoll_ctl returned -1 on adding client %d", c );
                }
                else
                {
                    if( ERR_NR == EAGAIN || ERR_NR == EWOULDBLOCK )
                        break;
                    printf( "Failed to accept connecting socket (code: %d)\n", ERR_NR );
                    break;
                }
            }
            continue;
        }
        else
        {
            int done = 0;

            map<SOCKET, ServerClient*>::iterator it = m_clients.find( m_events[ i ].data.fd );
            ServerClient* client = it != m_clients.end() ? it->second : NULL;

            if( !client )
                DBGMSG_F( "Fatal error, client %d is not in m_clients", m_events[ i ].data.fd );

            while( 1 )
            {
                ssize_t count = read( m_events[ i ].data.fd, m_buf, sizeof ( m_buf ) );
                if( count == -1 )
                {
                    if( ERR_NR != EAGAIN )
                    {
                        DBGMSG( "errno != EAGAIN at read" );
                        done = 1;
                    }
                    break;
                }
                else if( count == 0 )
                {
                    // eof, connection closed
                    //DBGMSG_F( "EOF on %d", m_events[i].data.fd );
                    done = 1;
                    break;
                }

                if( client )
                    OnClientData( client, (const unsigned char*)m_buf, count );
            }

            if( done )
            {
                //printf("Connection closed on socket %d\n",m_events[i].data.fd);
                closesocket( m_events[ i ].data.fd );
                if( client )
                {
                    m_clients.erase( it );
                    OnClientDisconnected( client );
                    FreeClient( client );
                }
            }
        }
    }
#else // __linux
    // windows implementation of select (we don't really care of high performance on windows as windows is only used for developing)
    FD_ZERO( &m_fd );
    FD_SET( m_sock, &m_fd );
    if( select( m_sock, &m_fd, NULL, NULL, &m_tv ) > 0 &&
        FD_ISSET( m_sock, &m_fd ) )
    {
        // accept a new client
        sockaddr_in addr;
        int addrlen = sizeof( addr );
        SOCKET c = accept( m_sock, (LPSOCKADDR)&addr, &addrlen );
        if( c != INVALID_SOCKET )
        {
            ServerClient* client = AllocateClient( c );
            GetIPAddress( &addr, client->szIP, sizeof ( client->szIP ) );
            m_clients.insert( pair<SOCKET, ServerClient*>( c, client ) );
            OnClientConnected( client );
        }
        else
            printf( "Invalid socket in accept (code: %u)\n", ERR_NR );
    }

    // loop through each client and see if there is data available
    for( map<SOCKET, ServerClient*>::iterator it = m_clients.begin(); it != m_clients.end(); )
    {
        ServerClient* client = it->second;

        if( client->bDisconnect )
        {
            it = m_clients.erase( it );

            if( shutdown( client->Socket, SHUT_RDWR ) == -1 )
                DBGMSG_F( "shutdown failed on client %d with error: %d", client->Socket, ERR_NR );
            closesocket( client->Socket );
            OnClientDisconnected( client );
            FreeClient( client );
        }
        else
        {
            FD_ZERO( &m_fd );
            FD_SET( client->Socket, &m_fd );
            if( select( client->Socket, &m_fd, NULL, NULL, &m_tv ) > 0 &&
                FD_ISSET( client->Socket, &m_fd ) )
            {
                // data receive
                int len = recv( client->Socket, m_buf, sizeof( m_buf ), 0 );
                if( len > 0 )
                    OnClientData( client, (const unsigned char*)m_buf, len );
                else
                    client->bDisconnect = true;
            }

            ++it;
        }
    }
#endif // __linux
}

ServerClient* CNetworkServer::AllocateClient( SOCKET Socket )
{
    ServerClient* client = new ServerClient( this );
    client->Init( Socket );
    return client;
}

void CNetworkServer::FreeClient( ServerClient* client )
{
    delete client;
}