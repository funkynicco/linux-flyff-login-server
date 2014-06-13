/*
 * File:   CertifyServer.cpp
 * Author: nicco
 *
 * Created on June 13, 2014, 09:52 AM
 */

#include "stdafx.h"
#include "CertifyServer.h"

#define HEADER_SIZE 13

#define DEFINE_FUNC( name ) void CCertifyServer::name( CertifyClient* client, CAr& ar )

/*
 * CertifyClient
 */

CertifyClient::CertifyClient( CCertifyServer* pServer ) :
ServerClient( pServer )
{
}

CertifyClient::~CertifyClient()
{
    // while each client is pooled, do not place vital code here for each session, use CertifyClient::Uninit()
}

void CertifyClient::Init( SOCKET Socket )
{
    ServerClient::Init( Socket ); // call base method first

    ProtocolId = (uint32_t)( (CCertifyServer*)pServer )->m_nNextId++;
    tmStartTime = time( NULL );
    bAuthenticated = false;
    *szAccount = 0;
}

void CertifyClient::Uninit()
{
    ServerClient::Uninit();

    // ...
}

/*
 * CertifyServer
 */

#define AddFunction( _header, _method ) m_functions.insert( pair<uint32_t, pfnFuncPtr>( _header, &CCertifyServer::_method ) );

CCertifyServer::CCertifyServer() :
m_pFreeList( NULL ),
m_bIsDestroy( false ),
m_nNextId( 1 ),
tmNextCheckClients( 0 ),
m_nAuthenicationTimeout( 30 )
{
    *m_szRequiredVersion = 0; // disabled by default
    bool bEnableRequiredVersion = false;

    if( !CConfiguration::GetInstance()->GetValue( "RequireVersion", &bEnableRequiredVersion ) )
        DBGMSG( "RequireVersion was not present in configuration file." );

    if( bEnableRequiredVersion &&
        !CConfiguration::GetInstance()->GetValue( "ClientVersion", m_szRequiredVersion, sizeof( m_szRequiredVersion ) ) )
        DBGMSG( "ClientVersion was not present in configuration file." );

    if( !CConfiguration::GetInstance()->GetValue( "AuthenticationTimeout", &m_nAuthenicationTimeout ) )
        DBGMSG( "AuthenticationTimeout was not present in configuration file." );

    AddFunction( PACKETTYPE_CERTIFY, OnCertify );
    AddFunction( PACKETTYPE_PING, OnPing );
    AddFunction( PACKETTYPE_CLOSE_EXISTING_CONNECTION, OnCloseExistingConnection );
    AddFunction( PACKETTYPE_KEEP_ALIVE, OnKeepAlive );
    AddFunction( PACKETTYPE_ERROR, OnError );
}

CCertifyServer::~CCertifyServer()
{
    m_bIsDestroy = true; // any subsequential calls to FreeClient will directly delete it instead of pushing to the m_pFreeList

    while( m_pFreeList )
    {
        auto ptr = m_pFreeList;
        m_pFreeList = m_pFreeList->next;
        delete ptr;
    }
}

ServerClient* CCertifyServer::AllocateClient( SOCKET Socket ) // this and FreeClient overrides base allocate client method for client pooling
{
    auto client = m_pFreeList;
    if( client )
        m_pFreeList = m_pFreeList->next;
    else
        client = new CertifyClient( this );

    client->Init( Socket );

    return client;
}

void CCertifyServer::FreeClient( ServerClient* client )
{
    client->Uninit();

    if( !m_bIsDestroy )
    {
        ( (CertifyClient*)client )->next = m_pFreeList;
        m_pFreeList = (CertifyClient*)client;
    }
    else
        delete (CertifyClient*)client;
}

void CCertifyServer::OnClientConnected( ServerClient* _client )
{
    auto client = (CertifyClient*)_client;
    printf( "[%d] Client connected from %s\n", client->Socket, client->szIP );

    client->SendProtocolId();
}

void CCertifyServer::OnClientDisconnected( ServerClient* _client )
{
    auto client = (CertifyClient*)_client;
    printf( "[%d] Client disconnected from %s\n", client->Socket, client->szIP );
}

void CCertifyServer::OnClientData( ServerClient* _client, const unsigned char* data, size_t length )
{
    auto client = (CertifyClient*)_client;
    //printf( "[%d] Client data from %s (%d bytes)\n", client->Socket, client->szIP, (int)length );

    client->buffer.SetOffset( client->buffer.GetLength() );
    client->buffer.Write( data, length );

    while( client->buffer.GetLength() >= HEADER_SIZE )
    {
        if( *client->buffer.GetBuffer() != PACKET_PREFIX )
        {
            printf( "Invalid packet prefix received from %u: 0x%02x\n", client->Socket, *data );
            client->Disconnect( "Invalid packet prefix" );
            return;
        }

        client->buffer.SetOffset( 1 );

        uint32_t checksum, size, dataChecksum;
        client->buffer >> checksum >> size >> dataChecksum;

        // check header checksum

        if( client->buffer.GetLength() - HEADER_SIZE >= size )
        {
            // read entire packet and parse and whatnot
            uint32_t header;
            client->buffer >> header;

            printf( "checksum:%08x, size:%08x, dataChecksum:%08x, header:%08x\n", checksum, size, dataChecksum, header );

            map<uint32_t, pfnFuncPtr>::iterator it = m_functions.find( header ); // lookup packet handler method
            if( it == m_functions.end() )
            {
                printf( "Invalid header received from %u: %08x\n", client->Socket, header );
                client->Disconnect( "Invalid header received" );
                return;
            }

            ( this->*it->second )( client, client->buffer ); // invoke packet method

            if( client->bDisconnect )
                return; // do not process more packets from this client if disconnected

            client->buffer.Remove( HEADER_SIZE + size );
        }
    }
}

void CCertifyServer::Process( time_t tm )
{
    CNetworkServer::Process( tm );

    if( tm >= tmNextCheckClients )
    {
        for( map<SOCKET, ServerClient*>::iterator it = m_clients.begin(); it != m_clients.end(); ++it )
        {
            auto client = (CertifyClient*)it->second;

            if( !client->bAuthenticated &&
                tm - client->tmStartTime > m_nAuthenicationTimeout )
            {
                client->Disconnect( "Authentication timed out" );
            }
        }

        tm = tmNextCheckClients + 2;
    }
}

/*
 * Handler methods ..
 */

DEFINE_FUNC( OnCertify )
{
    char client_version[ 128 ];
    char account[ MAX_ACCOUNT ];

    ar.ReadString( client_version, sizeof( client_version ) );
    ar.ReadString( account, sizeof( account ) );
    char szEnc[ 16 * MAX_PASSWORD ] = { 0 };
    char szDec[ 16 * MAX_PASSWORD ] = { 0 };

    memset( szDec, 0xcd, sizeof( szDec ) );

    ar.Read( szEnc, sizeof( szEnc ) );

    g_xRijndael->ResetChain();
    g_xRijndael->Decrypt( szEnc, szDec, sizeof( szEnc ), CRijndael::CBC );

    printf( "Auth request\n\tclient_version: %s\n\taccount: %s\n\tpassword: %s\n", client_version, account, szDec );

    client->SendError( ERROR_FLYFF_ACCOUNT );
}

DEFINE_FUNC( OnPing )
{

}

DEFINE_FUNC( OnCloseExistingConnection )
{

}

DEFINE_FUNC( OnKeepAlive )
{

}

DEFINE_FUNC( OnError )
{

}