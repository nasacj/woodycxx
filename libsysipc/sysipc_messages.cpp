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
#include "sysipc_messages.h"

namespace sysipc
{

byte parameter_list::null_tlv_buffer[ 129 ] = 
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

tlv parameter_list::null_tlv( null_tlv_buffer ) ;



//------------------------------------------------------------------------------------------------------------------
bool parameter_list::load( byte* payload, uint payload_size )
{
    ok = false;
    _size = payload_size;
    _space = 0;
    _cursor = payload;
    tlvs.clear();

    while ( _size > 0 && _size < sysipc::MAX_SYSTEM_MESSAGE_SIZE ) 
    {
        tlv t( _cursor );

        // an emty tlv or not enough some kind of length error terminates load
        if ( ( !t.is_null() )  && ( t.size() <= _size )  )
        {
            add_parameter( t );
            _cursor += t.size();
            _size -= t.size();
        }
        else
        {
            ok = false;
            return ok;
        }
    }
    ok = true;
    return ok;
}

//------------------------------------------------------------------------------------------------------------------
void parameter_list::add_byte_array( byte* v, byte nelems )
{
    byte n = nelems * 1;

    // _space > size rather that >= because need 1 byte for prefix
    if ( ( nelems  > 0 ) && ( nelems <= 16 ) && ( _space > n ) )
    {
        // No zero length elements, so remember 0 = 16 bytes
        // This will acually store prefix byte at _cursor
        byte_tlv t( _cursor++, n & tlv::INT_LENGTH_MASK  );
        add_parameter( t );

        // Now copy data 
        for ( uint i = 0; i < nelems ; i++ )
            t.set_value( *v++,  i );

        // adjust cursor (its length cause we bumped it above), space, size
        _cursor += t.length();
        _size += t.size();
        _space -= t.size();
    }
}

//------------------------------------------------------------------------------------------------------------------
byte_tlv parameter_list::add_byte_array( byte nelems )
{
    byte n = nelems * 1;

    // _space > size rather that >= because need 1 byte for prefix
    if ( ( nelems  > 0 ) && ( nelems <= 16 ) && ( _space > n ) )
    {
        // No zero length elements, so remember 0 = 16 bytes
        // This will acually store prefix byte at _cursor
        byte_tlv t( _cursor++, n & tlv::INT_LENGTH_MASK  );
        add_parameter( t );

        // Now zero out data
        for ( uint i = 0; i < nelems ; i++ )
            t.set_value( 0,  i );

        // adjust cursor (its length cause we bumped it above), space, size
        _cursor += t.length();
        _size += t.size();
        _space -= t.size();

        return t;
    }
    return byte_tlv();
}

//------------------------------------------------------------------------------------------------------------------
void parameter_list::add_word_array( word* v, byte nelems )
{
    byte n = nelems * 2;

    // _space > size rather that >= because need 1 byte for prefix
    if ( ( nelems  > 0 ) && ( nelems <= 8 ) && ( _space > n ) )
    {
        // No zero length elements, so remember 0 = 64 bytes
        // This will acually store prefix byte at _cursor
        word_tlv t( _cursor++, n & tlv::INT_LENGTH_MASK  );
        add_parameter( t );

        // Now copy data MSB first
        // Now copy data 
        for ( uint i = 0; i < nelems ; i++ )
            t.set_value( *v++,  i );

        // adjust cursor (its length cause we bumped it above), space, size
        _cursor += t.length();
        _size += t.size();
        _space -= t.size();
    }
}

//------------------------------------------------------------------------------------------------------------------
void parameter_list::add_dword_array( dword* v, byte nelems )
{
    byte n = nelems * 4;

    // _space > size rather that >= because need 1 byte for prefix
    if ( ( nelems  > 0 ) && ( nelems <= 4 ) && ( _space > n ) )
    {
        // No zero length elements, so remember 0 = 16 bytes
        // This will acually store prefix byte at _cursor.
        // Bump the cursor so it no points to where values go
        dword_tlv t( _cursor++, n & tlv::INT_LENGTH_MASK  );
        add_parameter( t );

        // Now copy data 
        for ( uint i = 0; i < nelems ; i++ )
            t.set_value( *v++,  i );

        // adjust cursor (its length cause we bumped it above), space, size
        _cursor += t.length();
        _size += t.size();
        _space -= t.size();
    }
}

//------------------------------------------------------------------------------------------------------------------
void parameter_list::add_string( const char* v )
{
    byte n = strlen(v);

    // We are not going to allow for zero length strings
    // _space > size rather that >= because need 1 byte for prefix
    if ( ( n  > 0 ) && ( n <= (tlv::STRING_LENGTH_MASK+1) ) && ( _space > n ) )
    {
        // No zero length elements, so remember 0 = 128 bytes
        // This will acually store prefix byte at _cursor
        string_tlv t( _cursor++, n & tlv::STRING_LENGTH_MASK  );
        add_parameter( t );

        // Now copy data
        t.set_value( v );

        // adjust cursor (its length cause we bumped it above), space, size
        _cursor += t.length();
        _size += t.size();
        _space -= t.size();
    }
}

//------------------------------------------------------------------------------------------------------------------
void parameter_list::add_string( const string& v )
{
    byte n = v.size();

    // We are not going to allow for zero length strings
    // _space > size rather that >= because need 1 byte for prefix
    if ( ( n  > 0 ) && ( n <= (tlv::STRING_LENGTH_MASK+1) ) && ( _space > n ) )
    {
        // No zero length elements, so remember 0 = 128 bytes
        // This will acually store prefix byte at _cursor
        string_tlv t( _cursor++, n & tlv::STRING_LENGTH_MASK  );
        add_parameter( t );

        // Now copy data
        t.set_value( v );

        // adjust cursor (its length cause we bumped it above), space, size
        _cursor += t.length();
        _size += t.size();
        _space -= t.size();
    }
}

//------------------------------------------------------------------------------------------------------------------
ostream& operator<<( ostream& strm, const parameter_list& parms )
{
    if ( parms.num_params() > 0 )
    {
        for ( uint i = 0; i < parms.num_params(); i++ )
        {   
            if ( i > 0 )
                strm << " ";
            strm << "[" << i << "] " << parms[ i ];
        }
    }
    return strm;
}

} // end namespace





