/*
 * File:   Configuration.h
 * Author: nicco
 *
 * Created on June 13, 2014, 2:30 PM
 */

#pragma once

enum ConfigurationValueType
{
    CVT_STRING,
    CVT_NUMBER,
    CVT_BOOLEAN
};

struct ConfigurationValue
{
    ConfigurationValueType cvtType;
    char szValue[ 256 ];
    size_t szValueLen;
    int32_t intValue;
};

class CConfiguration
{
public:
    CConfiguration();
    virtual ~CConfiguration();

    bool Load( const char* filename );
    bool CreateDefault( const char* filename );

    bool GetValue( const char* name, char* output, size_t sizeOfOutput );
    bool GetValue( const char* name, int32_t* pnNum );
    bool GetValue( const char* name, bool* pnBool );

    static CConfiguration* GetInstance()
    {
        static CConfiguration instance;
        return &instance;
    }

private:
    bool ParseConfiguration( const char* ptr_begin, const char* ptr_end );
    map<string, ConfigurationValue*> m_values;
};