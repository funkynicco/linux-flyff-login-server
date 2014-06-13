/* 
 * File:   stdafx.h
 * Author: nicco
 *
 * Created on June 12, 2014, 11:36 AM
 */

#pragma once

#include <iostream>
#include <time.h>

#ifdef __linux
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <netdb.h>
#include <cstdlib>
#include <fcntl.h>
#include <errno.h>
#endif // __linux

#ifdef _WIN32
#include <Windows.h>
#endif // _WIN32

#include <map>
#include <string>

using namespace std;

#ifndef _DEBUG
#define DBGMSG( __msg ) printf( "In method '%s' at %s:%d:\t%s\n", __FUNCTION__, __FILE__, __LINE__, __msg );
/*
 * Writes debug messages in console. This requires formatting parameters in the end.
 */
#define DBGMSG_F( __msg, ... ) \
{\
    char __intmsg[ 4096 ] = { 0 }; \
    sprintf( __intmsg, "In method '%s' at %s:%d:\t%s\n", __FUNCTION__, __FILE__, __LINE__, __msg ); \
    printf( __intmsg, __VA_ARGS__ ); \
}
#endif // _DEBUG

#include "Core/Preconfig.h"
#include "Core/ConsoleUtilities.h"
#include "Network/Utilities.inl"
#include "Network/NetworkServer.h"