/******************************************************************************
 * 
 *  Licensed Materials - Property of IBM.
 * 
 *  (C) Copyright IBM Corporation 2008
 * 
 *  All Rights Reserved.
 * 
 *  US Government Users Restricted Rights -
 *  Use, duplication or disclosure restricted by
 *  GSA ADP Schedule Contract with IBM Corporation.
 * 
 *****************************************************************************/
#include "sysipc_tlv.h"
#include <iomanip>

namespace sysipc
{

//----------------------------------------------------------------------------------------------------------------------
ostream& operator<<( ostream& strm, const byte_tlv& t )
{
    uint len = t.length();
    for ( uint i = 0; i < len; i++ )
        strm << setfill( '0' ) << setw( 2 ) << hex << (int) t.get_value( i ) << " ";
    return strm;
}

//----------------------------------------------------------------------------------------------------------------------
ostream& operator<<( ostream& strm, const word_tlv& t )
{
    uint len = t.length() >> 1;
    for ( uint i = 0; i < len; i++ )
        strm << setfill( '0' ) << setw( 4 ) << hex << (int) t.get_value( i ) << " ";
    return strm;
}

//----------------------------------------------------------------------------------------------------------------------
ostream& operator<<( ostream& strm, const dword_tlv& t )
{
    uint len = t.length() >> 2;
    for ( uint i = 0; i < len; i++ )
        strm << setfill( '0' ) << setw( 8 ) << hex << (int) t.get_value( i ) << " ";
    return strm;
}

//----------------------------------------------------------------------------------------------------------------------
ostream& operator<<( ostream& strm, const string_tlv& t )
{
    strm << t.get_value().c_str() << " ";
    return strm;
}

//----------------------------------------------------------------------------------------------------------------------
void string_tlv::set_value( const char* v )
{
    uint n = strlen( v );
    if ( length() < n )
        n = length();
    memcpy( _values, v, n );
}

//----------------------------------------------------------------------------------------------------------------------
void string_tlv::set_value( const string& v )
{
    uint n = v.size();
    if ( length() < n )
        n = length();
    memcpy( _values, v.data(), n );
}

//----------------------------------------------------------------------------------------------------------------------
string  string_tlv::get_value() const 
{ 
    if ( length() == 0 )
        return string();
    else
        return string( (char*)_values, length() );
 }

//----------------------------------------------------------------------------------------------------------------------
byte byte_tlv::get_value( uint i ) const
{
    return _values[ i ];
}

//----------------------------------------------------------------------------------------------------------------------
void byte_tlv::set_value( byte v, uint i )
{
    _values[ i ] = v;
}

//----------------------------------------------------------------------------------------------------------------------
void word_tlv::set_value( word v, uint i )
{
    byte*   p = _values + ( i << 1 );

    // MSB first
    *p++ = (byte) ( v >> 8 );
    *p++ = (byte) ( v );
}

//----------------------------------------------------------------------------------------------------------------------
word word_tlv::get_value( uint i ) const
{
    byte*   p = _values + ( i << 1 );

    // MSB first
    word v = *p++ << 8;
    v |= *p++;

    return v;
}

//----------------------------------------------------------------------------------------------------------------------
void dword_tlv::set_value( dword v, uint i )
{
    byte*   p = _values + ( i << 2 );

    // MSB first
    *p++ = (byte) ( v >> 24 );
    *p++ = (byte) ( v >> 16 );
    *p++ = (byte) ( v >> 8 );
    *p++ = (byte) ( v );
}

//----------------------------------------------------------------------------------------------------------------------
dword dword_tlv::get_value( uint i) const
{
    byte*   p = _values + ( i << 2 );

    // MSB first
    dword v = *p++ << 24;
    v |= *p++ << 16;
    v |= *p++ << 8;
    v |= *p++;

    return v;
}

//----------------------------------------------------------------------------------------------------------------------
ostream& operator<<( ostream& strm, const tlv& t )
{
    switch ( t.type() )
    {
        case tlv::byte_type:
            {
                strm << byte_tlv( t );
            }
                break;
    
        case tlv::word_type:
                {
                    strm << word_tlv( t );
                }
                break;
    
        case tlv::dword_type:
                {
                    strm << dword_tlv( t );
                }
                break;
    
        case tlv::string_type:
                {
                    strm << string_tlv( t );
                }
                break;
        default:
            break;
    
    }
    return strm;
}

byte tlv::get_length( byte prefix ) 
{
    if ( is_string( prefix ) )
    {
        byte rslt = STRING_LENGTH_MASK & prefix;
        if ( rslt == 0 )
            rslt = STRING_LENGTH_MASK + 1;
        return rslt; 
    }
    else
    {
        byte rslt = INT_LENGTH_MASK & prefix;
        if ( rslt == 0 )
            rslt = INT_LENGTH_MASK + 1;
        return rslt;
    }
}


} // end namespace
