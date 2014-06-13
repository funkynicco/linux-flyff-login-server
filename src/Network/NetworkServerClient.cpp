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