/*
 * File:   NetworkServerClient.cpp
 * Author: nicco
 *
 * Created on June 12, 2014, 11:58 AM
 */

#include "stdafx.h"
#include "NetworkServer.h"

ServerClient::ServerClient( CNetworkServer* pServer ) :
pTag( NULL )
{
    this->pServer = pServer;
#ifdef _DEBUG
    Init( INVALID_SOCKET );
#endif // _DEBUG
}

void ServerClient::Init( SOCKET Socket )
{
    this->Socket = Socket;
    bDisconnect = false;
}

int ServerClient::Send( const void* data, size_t len )
{
    return send( Socket, (const char*)data, (int)len, 0 );
}