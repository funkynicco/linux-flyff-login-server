/*
 * File:   MsgHdr.h
 * Author: nicco
 *
 * Created on June 13, 2014, 10:23 AM
 */

#pragma once

#define MAX_ACCOUNT                             42 // Maximum length an account name can be, note that this is WITH the ending \0
#define MAX_PASSWORD                            42

#define PACKET_PREFIX                           (unsigned char)0x5e

#define PACKETTYPE_PROTOCOL_ID                  (uint32_t)0x00000000
#define	PACKETTYPE_PING                         (uint32_t)0x00000014
#define	PACKETTYPE_CLOSE_EXISTING_CONNECTION    (uint32_t)0x00000016
#define	PACKETTYPE_KEEP_ALIVE                   (uint32_t)0x00000018
#define	PACKETTYPE_CERTIFY                      (uint32_t)0x000000fc
#define PACKETTYPE_ERROR                        (uint32_t)0x000000fe

// Authentication messages
#define ERROR_FLYFF_PASSWORD                    (uint32_t)120   // Wrong password
#define ERROR_FLYFF_ACCOUNT                     (uint32_t)121   // Wrong account name