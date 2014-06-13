/*
 * File:   CertifyServerClient.cpp
 * Author: nicco
 *
 * Created on June 13, 2014, 11:09 AM
 */

#include "stdafx.h"
#include "CertifyServer.h"

//#define __SHOW_SENDPACKET

void CertifyClient::SendPacket( CAr& ar )
{
    CAr ar2;
    ar2 << PACKET_PREFIX << (uint32_t)0;

    ar2.Write( ar.GetBuffer(), ar.GetLength() );

    ar2.SetOffset( 1 );
    ar2 << (uint32_t)ar2.GetLength() - 5;

#ifdef __SHOW_SENDPACKET
    printf( "Send %u bytes\n", ar2.GetLength() ); // should be 13 bytes for protocol id packet
    for( size_t i = 0; i < ar2.GetLength(); ++i )
    {
        if( i > 0 )
        {
            if( i % 24 == 0 )
                printf( "\n" );
            else
                printf( " " );
        }

        printf( "%02x", *( ar2.GetBuffer() + i ) );
    }
    printf( "\n" );
#endif // __SHOW_SENDPACKET

    uint32_t pos = 0;
    while( pos < (uint32_t)ar2.GetLength() )
    {
        int sent = Send( ar2.GetBuffer() + pos, ar2.GetLength() - pos );
        if( sent <= 0 )
        {
            printf( "CertifyClient::SendPacket failed to send all data to %u!\n", Socket );
            return;
        }

        pos += sent;
    }
}

void CertifyClient::SendProtocolId()
{
    CAr ar;
    ar << PACKETTYPE_PROTOCOL_ID << ProtocolId;
    SendPacket( ar );
}

void CertifyClient::SendError( uint32_t code )
{
    CAr ar;
    ar << PACKETTYPE_ERROR << code;
    SendPacket( ar );
}