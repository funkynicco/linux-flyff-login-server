/*
 * File:   CertifyServer.h
 * Author: nicco
 *
 * Created on June 13, 2014, 09:52 AM
 */

#pragma once

class CCertifyServer;
struct CertifyClient : ServerClient
{
    CertifyClient( CCertifyServer* pServer );
    virtual ~CertifyClient();
    virtual void Init( SOCKET Socket );
    virtual void Uninit();

    uint32_t ProtocolId;
    CAr buffer;
    CertifyClient* next;
    time_t tmStartTime; // the time when the session was started, if this exceeds a specified amount the client will be disconnected (certify time-out)
    char szAccount[ MAX_ACCOUNT ];
    bool bAuthenticated;

    void SendPacket( CAr& ar );
    void SendProtocolId();
    void SendError( uint32_t code );
};

#define DECLARE_FUNC( name ) void name( CertifyClient* client, CAr& ar );

typedef void( CCertifyServer::* pfnFuncPtr)( CertifyClient* client, CAr& ar );

class CCertifyServer : public CNetworkServer
{
public:
    CCertifyServer();
    virtual ~CCertifyServer();
    virtual void Process( time_t tm );

protected:
    virtual ServerClient* AllocateClient( SOCKET Socket );
    virtual void FreeClient( ServerClient* client );
    virtual void OnClientConnected( ServerClient* client );
    virtual void OnClientDisconnected( ServerClient* client );
    virtual void OnClientData( ServerClient* client, const unsigned char* data, size_t length );

    DECLARE_FUNC( OnCertify );
    DECLARE_FUNC( OnPing );
    DECLARE_FUNC( OnCloseExistingConnection );
    DECLARE_FUNC( OnKeepAlive );
    DECLARE_FUNC( OnError );

private:
    friend struct CertifyClient;
    CertifyClient* m_pFreeList;
    bool m_bIsDestroy; // if this is true then each client sent to FreeClient will be deleted directly because the pool is cleared
    unsigned long long m_nNextId;
    map<uint32_t, pfnFuncPtr> m_functions;
    time_t tmNextCheckClients;
};

#undef DECLARE_FUNC