/* 
 * File:   NetworkServer.h
 * Author: nicco
 *
 * Created on June 12, 2014, 11:57 AM
 */

#pragma once

#define MAX_BUFFER 32768

class CNetworkServer;

struct ServerClient
{
    CNetworkServer* pServer;
    SOCKET Socket;
    char szIP[16];
    bool bDisconnect;
    char szDisconnectReason[ 256 ];
    void* pTag;

    ServerClient( CNetworkServer* pServer );

    // add other shits here
    virtual void Init( SOCKET Socket );

    virtual void Uninit( )
    {
    }

    void Disconnect( const char* reason = NULL )
    {
        bDisconnect = true;
        if( reason )
        {
            if( strlen( reason ) + 1 > sizeof (szDisconnectReason) )
            {
                DBGMSG_F( "Reason is too long. Max %u characters.", sizeof (szDisconnectReason) - 1 );
                return;
            }

            strcpy( szDisconnectReason, reason );
        }
    }
};

class CNetworkServer
{
public:
    CNetworkServer( );
    virtual ~CNetworkServer( );

    bool Start( uint16_t port );
    void Close( );
    void Process( time_t tm );

protected:

    virtual void OnClientConnected( ServerClient* client )
    {
    }

    virtual void OnClientDisconnected( ServerClient* client )
    {
    }

    virtual void OnClientData( ServerClient* client, const unsigned char* data, size_t length )
    {
    }

    virtual ServerClient* AllocateClient( SOCKET Socket );
    virtual void FreeClient( ServerClient* client );

private:
    SOCKET m_sock;
#ifdef __linux
    struct epoll_event m_event;
    struct epoll_event* m_events;
    int m_efd;
#else // __linux
    fd_set m_fd; // simple FD set on windows
    TIMEVAL m_tv;
#endif // __linux
    map<SOCKET, ServerClient*> m_clients;
    char m_buf[ MAX_BUFFER ];
};
