/* 
 * File:   preconfig.h
 * Author: nicco
 *
 * Created on June 12, 2014, 12:03 PM
 */

#pragma once

#ifdef __linux // Define things for Linux here
/*
 * Represents a socket identifier.
 */
#define SOCKET int
/*
 * Represents an invalid socket.
 */
#define INVALID_SOCKET ((int)-1)
/*
 * Represents a socket error.
 * Socket related methods may return this value, such as bind, listen, recv, send, etc.
 */
#define SOCKET_ERROR ((int)-1)
/*
 * Provides a millisecond sleep interface for Linux that uses usleep.
 */
#define Sleep( __ms ) usleep( __ms * 1000 )
#define VK_ESCAPE 0x1B
#define closesocket( __sock ) close( __sock )

#define ERR_NR errno
#endif // __linux

#ifdef _WIN32 // Define things for Windows here
#define ERR_NR GetLastError()
#define SHUT_RDWR SD_BOTH
#define SHUT_RD SD_RECEIVE
#define SHUT_WR SD_SEND
#endif // _WIN32