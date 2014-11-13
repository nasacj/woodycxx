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
#ifndef __SYSIPC_MESSAGES_H
#define __SYSIPC_MESSAGES_H

//------------------------------------------------------------------------------------------------------------------
/// \ingroup sysipc
/// \file sysipc.h
/// \brief Messages
//------------------------------------------------------------------------------------------------------------------
#include "sysipc.h"
#include "sysipc_tlv.h"

//----------------------------------------------------------------------------------------------------------------------
/// \ingroup sysipc
/// \brief Message Types
const long MESSAGE_FM           = 1;
const long MESSAGE_APP_CONTROL  = 2;
const long MESSAGE_SP           = 3;
const long MESSAGE_CM           = 4;
const long MESSAGE_CMR          = 5;
const long MESSAGE_TKLM         = 6;
const long MESSAGE_DP           = 7;
const long MESSAGE_PLDM         = 8;

namespace sysipc
{

//----------------------------------------------------------------------------------------------------------------------
/// \ingroup sysipc
/// \brief Message Parameter List
class parameter_list
{
    bool    ok;
    byte*   _cursor;
    uint    _space;
    uint    _size;

    vector<tlv>      tlvs;

    void add_parameter( tlv& t )
    {
        tlvs.push_back( t );
    }

    static byte null_tlv_buffer[ 129 ];
    static tlv null_tlv;

    friend ostream& operator<<( ostream& strm, const parameter_list& );

protected:
    /// \brief Empties current contents and prepares to add parameters
    void clear( byte* payload, uint payload_capacity )
    { 
        ok = false;
        _cursor = payload;
        _space = payload_capacity;
        _size = 0;
        tlvs.clear();
    }

    /// \brief Loads clears in preparation for parsing
    bool load( byte* payload, uint payload_size );

public:
    parameter_list()
    { 
        clear( 0, 0 );
    }

    bool is_ok() const { return ok; }
    uint space() const { return _space; }
    uint size() const { return _size; }

    void        add_byte( byte v ){ add_byte_array( &v, 1 ); }
    void        add_byte_array( byte* v, byte nelems );
    byte_tlv    add_byte_array( byte nelems );

    void        add_word( word v ){ add_word_array( &v, 1 ); }
    void        add_word_array( word* v, byte nelems );
    word_tlv    add_word_array( byte nelems );

    void        add_dword( dword v ){ add_dword_array( &v, 1 ); }
    void        add_dword_array( dword* v, byte nelems );
    dword_tlv   add_dword_array( byte nelems );

    void        add_string( const char* v );
    void        add_string( const string& v );


    //bool load();
    const tlv& operator[]( uint i ) const 
    { 
        if ( i < tlvs.size() )
            return tlvs[ i ];
        else
            return null_tlv;
    }
    uint num_params() const { return (uint)tlvs.size(); }
};
ostream& operator<<( ostream& strm, const parameter_list& );

//------------------------------------------------------------------------------------------------------------------
/// \ingroup sysipc
/// \brief Flash Manager Command
template<class header_class>
class Tmessage
{
    header_class*               _header;
    uint                        _capacity;

public:
    Tmessage( byte* buffer = 0, uint in_capacity = 0 )
    { 
        init( buffer, in_capacity );
    }
    bool                init( byte* buffer, uint buffer_size )
                        {
                            _header = (header_class*) buffer;
                            _capacity = buffer_size;
                            return !( ( buffer_size < header_size() ) ||  ( buffer_size < size() ) );
                        }
    uint                size() const { return payload_size() + header_size(); }
    uint                capacity() const { return _capacity; }
    uint                space() const { return capacity() - size(); }
    header_class&       get_header(){ return *_header; }
    const header_class& get_header() const { return *_header; }
    static uint         header_size(){ return sizeof(header_class); } 
    uint                payload_size() const { return get_header().payload_size(); }
    byte*               payload(){ return message() + header_size(); }
    const byte*         payload() const{ return message() + header_size(); }
    uint                payload_capacity() const { return capacity() - header_size(); }
    const byte*         message() const { return (byte*)_header; }
    byte*               message(){ return (byte*)_header; }
    bool                is_overrun() const { return size() > capacity(); }
    uint                copy_from( byte* buffer, uint n ) const
    {
        if ( _capacity < n )
            n = _capacity;
        memcpy( buffer, message(), n );
        return n;
    }
    uint                set_payload( byte* buffer, uint n )
    {
        if ( payload_capacity() < n )
            n = payload_capacity();
        memcpy( payload(), buffer, n );
        _header->size = n;
        return n;
    }

};                         

//------------------------------------------------------------------------------------------------------------------
/// \ingroup sysipc
/// \brief System Message Header
struct system_message_header
{
    long    mtype;
    byte    sender;
    byte    receiver;
    byte    size;
    byte    seq_number;

    system_message_header() :
        mtype( 0 ), sender( 0 ), receiver( 0 ), size( 0 ), seq_number( 0 ){}

    system_message_header( long typ, byte sndr, byte rcvr ) :
        mtype( typ ), sender( sndr ), receiver( rcvr ), size( 0 ), seq_number( 0 ){}

    void    init_response( byte sndr, const system_message_header& hdr )
    {
         mtype = hdr.mtype;
         receiver = hdr.sender;
         size = 0;
         sender = sndr;
         seq_number = hdr.seq_number;
    }

    void init_request( long mt, byte sndr, byte rcvr )
    {
         mtype = mt;
         receiver = rcvr;
         size = 0;
         sender = sndr;
         seq_number = 0;
    }

    byte    payload_size() const { return size; }
    byte&   payload_size() { return size; }
};

/// \ingroup sysipc
/// \brief System Message Type
/// 
const uint MAX_SYSTEM_MESSAGE_SIZE = 256;


//------------------------------------------------------------------------------------------------------------------
/// \ingroup sysipc
/// \brief System Message
class system_message : public Tmessage<system_message_header>
{
public:
   system_message( byte* buffer = 0, uint in_capacity = 0 ) :
      Tmessage<system_message_header>( buffer, in_capacity ){}
};

//------------------------------------------------------------------------------------------------------------------
template<class header_class>
class Tparameter_list : public parameter_list
{
public:
    Tparameter_list(){}
    void clear( Tmessage<header_class>& msg )
    {
       parameter_list::clear( msg.payload(), msg.payload_capacity() ); 
    }
    bool load( Tmessage<header_class>& msg )
    {
       return parameter_list::load( msg.payload(), msg.payload_size() ); 
    }
    bool load( system_message& msg )
    {
       return parameter_list::load( msg.payload(), msg.payload_size() ); 
    }
};


//----------------------------------------------------------------------------------------------------------------------
/// \ingroup sysipc
/// \brief Format Messages
template<class T>
class Tmessage_formatter
{
protected:
    Tmessage<T>         msg;
    Tparameter_list<T>  parameters;
    bool                ok;

public:
    Tmessage_formatter() : ok( true ){}

    const   Tmessage<T>& get_message() const { return msg; }
    void    finish( ostream* s = 0 )
    {
        msg.get_header().length = (byte)parameters.size();
    }

    bool    is_ok() const { return ok; }

    void init( byte* buffer, byte buffer_size )
    {
        ok = true;
        msg.init( buffer, buffer_size );
        parameters.clear( msg );
    }

    void init( system_message& sm )
    {
        ok = true;
        msg.init( sm.payload(), sm.payload_capacity() );
        parameters.clear( msg );
    }
};


}

#endif
