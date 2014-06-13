/*
 * File:   Configuration.cpp
 * Author: nicco
 *
 * Created on June 13, 2014, 2:30 PM
 */

#include "stdafx.h"
#include "Configuration.h"

#include <fstream>

#define MAX_FILE_BUFFER 32768

CConfiguration::CConfiguration()
{
}

CConfiguration::~CConfiguration()
{
    for( map<string, ConfigurationValue*>::iterator it = m_values.begin(); it != m_values.end(); ++it )
        delete it->second;
}

bool CConfiguration::Load( const char* filename )
{
    for( map<string, ConfigurationValue*>::iterator it = m_values.begin(); it != m_values.end(); ++it )
        delete it->second;
    m_values.clear();

    FILE* fp = fopen( filename, "rb" );
    if( !fp )
    {
        DBGMSG_F( "fopen file '%s' failed with code: %u", filename, ERR_NR );
        return false;
    }

    fseek( fp, 0, SEEK_END );
    long size = ftell( fp );

    if( size == 0 )
    {
        fclose( fp );
        DBGMSG_F( "Configuration file '%s' appears to be empty.", filename );
        return false;
    }

    fseek( fp, 0, SEEK_SET );

    static char buf[ MAX_FILE_BUFFER ];
    char* ptr_begin = buf;
    if( size + 1 > sizeof( buf ) ) // load in predefined buffer if we can, otherwise malloc below
        ptr_begin = (char*)malloc( size + 1 );

    // read
    long pos = 0;
    while( pos < size )
    {
        size_t len = fread( ptr_begin + pos, 1, size - pos, fp );

        if( len == 0 )
        {
            fclose( fp );
            if( ptr_begin != buf )
                free( ptr_begin );
            DBGMSG_F( "fread returned 0 while loading file '%s', pos: %u, size: %u", filename, pos, size );
            return false;
        }

        pos += len;
    }

    fclose( fp );

    *( ptr_begin + size ) = 0;

    bool result = ParseConfiguration( ptr_begin, ptr_begin + size );

    if( ptr_begin != buf )
        free( ptr_begin );

    return result;
}

bool CConfiguration::CreateDefault( const char* filename )
{
    ofstream fs( filename, ofstream::binary );
    if( fs.fail() )
        return false;

    const char* FileContents =
        "# Certifier configuration file\r\n"
        "\r\n"
        "# The server port defined here is only used if none is provided in command line parameters.\r\n"
        "ServerPort = 23000\r\n"
        "\r\n"
        "# The client version requirement settings.\r\n"
        "ClientVersion = 20131016\r\n"
        "RequireVersion = yes\r\n"
        "\r\n"
        "# Amount of seconds a client can linger unauthorized before being disconnected. 0 means indefinitely.\r\n"
        "AuthenticationTimeout = 30";

    fs.write( FileContents, strlen( FileContents ) );
    fs.close();

    return true;
}

bool CConfiguration::GetValue( const char* name, char* output, size_t sizeOfOutput )
{
    static char buf[ 256 ];
    strcpy( buf, name );
    ToLower( buf );

    map<string, ConfigurationValue*>::iterator it = m_values.find( buf );
    if( it == m_values.end() )
        return false;

    if( it->second->szValueLen + 1 > sizeOfOutput )
        return false;

    strcpy( output, it->second->szValue );
    return true;
}

bool CConfiguration::GetValue( const char* name, int32_t* pnNum )
{
    static char buf[ 256 ];
    strcpy( buf, name );
    ToLower( buf );

    map<string, ConfigurationValue*>::iterator it = m_values.find( buf );
    if( it == m_values.end() )
        return false;

    if( it->second->cvtType != CVT_NUMBER )
        return false;

    *pnNum = it->second->intValue;
    return true;
}

bool CConfiguration::GetValue( const char* name, bool* pnBool )
{
    static char buf[ 256 ];
    strcpy( buf, name );
    ToLower( buf );

    map<string, ConfigurationValue*>::iterator it = m_values.find( buf );
    if( it == m_values.end() )
        return false;

    if( it->second->cvtType != CVT_NUMBER &&
        it->second->cvtType != CVT_BOOLEAN )
        return false;

    *pnBool = it->second->intValue != 0;
    return true;
}

inline bool IsWhitespace( char cb )
{
    switch( cb )
    {
    case ' ':
    case '\t':
        return true;
    }

    return false;
}

inline void SkipWhitespace( const char* ptr_begin, const char* ptr_end, const char*& ptr )
{
    while( ptr < ptr_end &&
        IsWhitespace( *ptr ) )
        ++ptr;
}

bool CConfiguration::ParseConfiguration( const char* ptr_begin, const char* ptr_end )
{
    static char temp[ 1024 ];
    static char temp2[ 1024 ];
    const char* ptr = ptr_begin;
    int line = 1;

    while( ptr < ptr_end )
    {
        if( *ptr == '#' )
        {
            ++ptr;
            while( ptr < ptr_end )
            {
                if( *ptr == '\n' )
                {
                    ++line;
                    ++ptr;
                    break;
                }
                ++ptr;
            }
            continue;
        }
        else if( isalpha( *ptr ) )
        {
            char* tmp = temp;
            *tmp++ = *ptr++;
            while( ptr < ptr_end &&
                isalpha( *ptr ) &&
                tmp + 1 < temp + sizeof( temp ) )
                *tmp++ = *ptr++;
            *tmp = 0;

            SkipWhitespace( ptr_begin, ptr_end, ptr );

            if( *ptr != '=' )
            {
                printf( "Corrupted conf at line %d, unexpected character: %c (execpted =)\n", line, *ptr );
                return false;
            }
            ++ptr;

            SkipWhitespace( ptr_begin, ptr_end, ptr );

            tmp = temp2;
            while( ptr < ptr_end &&
                ( isalpha( *ptr ) || isdigit( *ptr ) || IsWhitespace( *ptr ) ) &&
                tmp + 1 < temp2 + sizeof( temp2 ) )
                *tmp++ = *ptr++;

            // remove trailing whitespaces
            while( tmp - 1 > temp &&
                IsWhitespace( *( tmp - 1 ) ) )
                --tmp;

            *tmp = 0;

            if( *temp2 == 0 )
            {
                printf( "Corrupted conf at line %d, no value for configuration key '%s'\n", line, temp );
                return false;
            }

            ToLower( temp );

            if( m_values.find( temp ) == m_values.end() )
            {
                ConfigurationValue* cv = new ConfigurationValue;

                strcpy( cv->szValue, temp2 );
                cv->szValueLen = strlen( cv->szValue );

                int32_t booleanValue = -1;
                if( strcmp( cv->szValue, "yes" ) == 0 )
                    booleanValue = 1;
                else if( strcmp( cv->szValue, "no" ) == 0 )
                    booleanValue = 0;

                if( booleanValue == -1 )
                {
                    int v = 0;
                    if( sscanf( temp2, "%d", &v ) > 0 )
                        cv->cvtType = CVT_NUMBER;
                    else
                        cv->cvtType = CVT_STRING;
                    cv->intValue = v;
                }
                else
                {
                    cv->intValue = booleanValue;
                    cv->cvtType = CVT_BOOLEAN;
                }

                m_values.insert( pair<string, ConfigurationValue*>( temp, cv ) );
            }
            else
                printf( "Warning: Configuration key '%s' already exist. Line: %d\n", temp, line );
        }
        else
        {
            switch( *ptr )
            {
            case ' ':
            case '\t':
            case '\r':
                break;
            case '\n':
                ++line;
                break;
            default:
                printf( "Corrupt conf at line %d, invalid character: %c\n", line, *ptr );
                return false;
            }
        }

        ++ptr;
    }

    return true;
}