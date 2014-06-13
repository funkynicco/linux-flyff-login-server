/*
 * File:   main.cpp
 * Author: nicco
 *
 * Created on June 12, 2014, 11:35 AM
 */

#include "stdafx.h"

int main( int argc, char** argv )
{
    int port = 23000;
    if( argc > 1 )
    {
        port = atoi( argv[ 1 ] );
        if( port <= 0 || port > 65535 )
        {
            printf( "Invalid port number %d given in parameter.\n", port );
            return 1;
        }
    }

    CAr faf;

#ifdef _WIN32
    WSADATA wd;
    int n;
    if( ( n = WSAStartup( MAKEWORD( 2, 2 ), &wd ) ) != 0 )
    {
        printf( "WSAStartup failed with code: %u\n", n );
        return 1;
    }
#endif // _WIN32

    CCertifyServer server;

    printf( "Starting server on port %d ...", port );
    if( server.Start( port ) )
    {
        printf( "\t[OK]\n" );

        while( 1 )
        {
            time_t tm = time( NULL );
            server.Process( tm );

            if( _kbhit() )
            {
                int ch = _getch();
                if( ch == VK_ESCAPE )
                    break;
            }

            Sleep( 25 );
        }

        printf( "Shutting down ...\n" );
        server.Close();
    }
    else
        printf( "\t[FAIL]\nFailed to start server on the port number provided.\n" );

#ifdef __linux
    RestoreTermIOS(); // Restores the console echo feedback on application exit
#endif // __linux
#ifdef _WIN32
    WSACleanup();
#endif // _WIN32
    return 0;
}

