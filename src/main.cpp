/*
 * File:   main.cpp
 * Author: nicco
 *
 * Created on June 12, 2014, 11:35 AM
 */

#include "stdafx.h"

class CTestServer : public CNetworkServer
{
public:
    CTestServer()
    {
    }

    virtual ~CTestServer()
    {
    }

protected:
    void OnClientConnected( ServerClient* client )
    {
        printf( "[%d] Client connected from %s\n", client->Socket, client->szIP );

        const char* html = "<h1>Hello world</h1><p>This is a test.</p>";

        char buf[ 4096 ];
        sprintf(
            buf,
            "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %d\r\n\r\n%s",
            (int)strlen( html ),
            html );

        send( client->Socket, buf, strlen( buf ), 0 );
    }

    void OnClientDisconnected( ServerClient* client )
    {
        printf( "[%d] Client disconnected from %s\n", client->Socket, client->szIP );
    }

    void OnClientData( ServerClient* client, const unsigned char* data, size_t length )
    {
        printf( "[%d] Client data from %s (%d bytes)\n", client->Socket, client->szIP, (int)length );

        return; // temp, dont print out all data ...

        static char buf[ 4096 ];
        size_t total = 0;
        while( length > 0 )
        {
            size_t l = min( sizeof ( buf ), length );
            memcpy( buf, data, l );
            data += l;
            length -= l;

            for( size_t i = 0; i < l; ++i )
            {
                if( total > 0 &&
                    total++ % 32 == 0 )
                    printf( "\n" );

                char cb = buf[ i ];
                if( IsValidChar( cb ) )
                    printf( "%c", cb );
                else
                    printf( "%%%02X", cb );
            }
        }
        printf( "\n" );
    }
};

int main( int argc, char** argv )
{
    int port = 80;
    if( argc > 1 )
    {
        port = atoi( argv[ 1 ] );
        if( port <= 0 || port > 65535 )
        {
            printf( "Invalid port number %d given in parameter.\n", port );
            return 1;
        }
    }

#ifdef _WIN32
    WSADATA wd;
    int n;
    if( ( n = WSAStartup( MAKEWORD( 2, 2 ), &wd ) ) != 0 )
    {
        printf( "WSAStartup failed with code: %u\n", n );
        return 1;
    }
#endif // _WIN32

    CTestServer server;

    if( server.Start( port ) )
    {
        printf( "Started server on port %d\n", port );

        while( 1 )
        {
            time_t tm = time( NULL );
            server.Process( tm );

            if( _kbhit() )
            {
                int ch = _getch();
                if( ch == VK_ESCAPE )//VK_ESCAPE
                    break;
            }

            Sleep( 25 );
        }

        printf( "Shutting down ...\n" );
        server.Close();
    }
    else
        printf( "Failed to start server\n" );

#ifdef __linux
    RestoreTermIOS(); // Restores the console echo feedback on application exit
#endif // __linux
#ifdef _WIN32
    WSACleanup();
#endif // _WIN32
    return 0;
}

