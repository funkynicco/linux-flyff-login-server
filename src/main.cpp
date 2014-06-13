/*
 * File:   main.cpp
 * Author: nicco
 *
 * Created on June 12, 2014, 11:35 AM
 */

#include "stdafx.h"

void PrintUsage( int argc, char** argv );

int main( int argc, char** argv )
{
    if( argc <= 1 )
    {
        PrintUsage( argc, argv );
        return 1;
    }

    int port = -1;
    char szConfigFile[ 256 ] = "certifier.conf";

    for( int i = 1; i < argc; ++i )
    {
        if( strcmp( argv[ i ], "-p" ) == 0 ||
            strcmp( argv[ i ], "--port" ) == 0 )
        {
            if( i + 1 < argc )
            {
                port = atoi( argv[ ++i ] );
                if( port <= 0 || port > 65535 )
                {
                    printf( "Invalid port number %d given in parameter.\n", port );
                    return 1;
                }
            }
            else
            {
                PrintUsage( argc, argv );
                return 1;
            }
        }
        else if(
            strcmp( argv[ i ], "-cfg" ) == 0 ||
            strcmp( argv[ i ], "--config" ) == 0 )
        {
            if( i + 1 < argc )
            {
                strcpy( szConfigFile, argv[ ++i ] );
            }
            else
            {
                PrintUsage( argc, argv );
                return 1;
            }
        }
        else if( strcmp( argv[ i ], "--makecfg" ) == 0 )
        {
            if( i + 1 < argc )
            {
                if( CConfiguration::GetInstance()->CreateDefault( argv[ ++i ] ) )
                    printf( "Configuration file '%s' created successfully.\n", argv[ i ] );
                else
                    printf( "Failed to create configuration file '%s'\n", argv[ i ] );

                return 0;
            }
            else
            {
                PrintUsage( argc, argv );
                return 1;
            }
        }
        else
        {
            printf( "Invalid parameter token '%s'\n", argv[ i ] );
            return 1;
        }
    }

    if( !CConfiguration::GetInstance()->Load( szConfigFile ) )
    {
        printf( "Failed to load configuration, make sure the file exist and is not corrupted.\n" );
        return 1;
    }

    if( port == -1 &&
        !CConfiguration::GetInstance()->GetValue( "ServerPort", &port ) )
    {
        printf( "Failed to get ServerPort value from configuration file\n" );
        return 1;
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

void PrintUsage( int argc, char** argv )
{
    printf( "Usage: %s [options]\n\n", argv[ 0 ] );
    printf( "Options:\n" );
    printf( "\t%-16s\t%s\n", "-p <port>", "" );
    printf( "\t%-16s\t%s\n", "--port <port>", "Set the port number to use." );
    printf( "\n" );
    printf( "\t%-16s\t%s\n", "-cfg <path>", "" );
    printf( "\t%-16s\t%s\n", "--config <path>", "Set the path to the configuration file to load." );

    printf( "\n" );
    printf( "\t%-16s\t%s\n", "--makecfg <path>", "Creates a default conf file." );
}