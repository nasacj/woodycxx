/******************************************************************************
 * 
 *  Licensed Materials - Property of IBM.
 * 
 *  (C) Copyright IBM Corporation 2007
 * 
 *  All Rights Reserved.
 * 
 *  US Government Users Restricted Rights -
 *  Use, duplication or disclosure restricted by
 *  GSA ADP Schedule Contract with IBM Corporation.
 * 
 *****************************************************************************/
#ifndef __SYSIPC_TLV_H
#define __SYSIPC_TLV_H

#include "sysipc.h"
#include <string.h>
#include <signal.h>
#include <vector>

/// \file sysipc_tlv.h
/// \brief Ttpe-Length-Value Encoding

namespace sysipc
{

//----------------------------------------------------------------------------------------------------------------------
/// \ingroup sysipc
/// \brief Type-Length-Vector
class tlv
{
    friend ostream& operator<<( ostream& strm, const tlv& t );

public:
    enum types
    {
        null_type   = 0x00,
        byte_type   = 0x10,
        word_type   = 0x20,
        dword_type  = 0x40,
        string_type = 0x80
    };

    static const byte TYPE_MASK             = 0x80;
    static const byte STRING_LENGTH_MASK    = 0x7F;
    static const byte INT_LENGTH_MASK       = 0x0F;

protected:
    tlv::types      _type;
    byte            _length;
    byte*           _values;

public:
    tlv() : 
         _type( null_type ), 
         _length( 0 ),
         _values( 0 ){}

    tlv( byte* p ) : 
         _type( get_type( *p) ), 
         _length( get_length( *p ) ),
         _values( p+1 )
    {

    }

    tlv( byte* p, tlv::types typ, byte len  )
    {
        *p = make_prefix( typ, len );
        _type = typ;
        _length = get_length( *p );
        _values = p+1;
    }

    static byte make_prefix( types typ, byte len ) 
    {
        if ( typ == string_type )
            return ( (byte)typ ) | ( STRING_LENGTH_MASK & len ); 
        else
            return ( (byte)typ ) | ( INT_LENGTH_MASK & len ); 
    }

    static types get_type( byte prefix ) 
    {
        if ( is_string( prefix ) )
            return ( types )( prefix & ~STRING_LENGTH_MASK ); 
        else
            return ( types )( prefix & ~INT_LENGTH_MASK ); 
    }

    static byte get_length( byte prefix ); 

    bool        is_string() const { return _type == string_type; }
    bool        is_null() const { return _type == null_type; }
    static bool is_string( byte prefix ) 
	{ 
		return string_type == ( (types)( prefix & ~STRING_LENGTH_MASK ) );
	}

    byte        size() const { return _length + 1; }
    byte        length() const { return _length; }
    const byte* values() const { return _values; }
    byte*       values() { return _values; }
    types       type() const { return _type; }
    tlv*        next() { return reinterpret_cast<tlv*>( _values + length() ); }
};
ostream& operator<<( ostream& strm, const tlv& t );


//----------------------------------------------------------------------------------------------------------------------
/// \ingroup sysipc
/// \brief Typed tlv class
class byte_tlv : public tlv
{
    
public:
    byte_tlv(){}
    byte_tlv( const tlv& t ) : tlv( t ){}
    byte_tlv( byte* p, byte len ) :
        tlv( p, byte_type, len ) {}

    byte        get_value( uint i = 0 ) const;
    void        set_value( byte v, uint i );
    byte        operator[]( uint i ) const 
                { 
                    return get_value( i );
                }
};

//----------------------------------------------------------------------------------------------------------------------
/// \ingroup sysipc
/// \brief Typed tlv class
class word_tlv : public tlv
{
    
public:
    word_tlv(){}
    word_tlv( const tlv& t ) : tlv( t ){}
    word_tlv( byte* p, byte len ) :
    tlv( p, word_type, len )
    {}

    word        get_value( uint i = 0 ) const;
    void        set_value( word v, uint i );
    word        operator[]( uint i ) const 
                { 
                    return get_value( i );
                }
};

//----------------------------------------------------------------------------------------------------------------------
/// \ingroup sysipc
/// \brief Typed tlv class
class dword_tlv : public tlv
{
    
public:
    dword_tlv(){}
    dword_tlv( const tlv& t ) : tlv( t ){}
    dword_tlv( byte* p, byte len ) :
        tlv( p, dword_type, len ) {}

    dword        get_value( uint i = 0 ) const;
    void         set_value( dword v, uint i );
    dword        operator[] ( uint i ) const 
                { 
                    return get_value( i );
                }
};

//----------------------------------------------------------------------------------------------------------------------
/// \ingroup sys_ipc
/// \brief Typed tlv class
class string_tlv : public tlv
{
public:

    string_tlv(){}
    string_tlv( const tlv& t ) :
        tlv( t ){}
    string_tlv( byte* p, byte len  ) :
        tlv( p, string_type, len  ){}

    string  get_value() const;
    void    set_value( const char* v );
    void    set_value( const string& v );

};

} // end namespace


#endif
