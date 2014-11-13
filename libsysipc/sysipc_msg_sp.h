/******************************************************************************
 * 
 *  Licensed Materials - Property of IBM.
 * 
 *  (C) Copyright IBM Corporation 2009
 * 
 *  All Rights Reserved.
 * 
 *  US Government Users Restricted Rights -
 *  Use, duplication or disclosure restricted by
 *  GSA ADP Schedule Contract with IBM Corporation.
 * 
 *****************************************************************************/
#ifndef __SYSIPC_MSG_SP_H
#define __SYSIPC_MSG_SP_H

#include "sysipc_message_queue.h"
#include <iomanip>

/* messaging protocol for use with the SP application */

namespace sysipc
{

class msg_sp
{
public:
    enum opcodes
    {
        op_test         = 0x00,
        op_invalid      = 0xFF
    };
    
    enum states
    {
        st_init         = 0x00,
        st_invalid      = 0xFF
    };

    struct header
    {
        byte    opcode;
        byte    length;
        
        byte    payload_size() const { return length; }
        byte&   payload_size() { return length; }
        
        header() : opcode(op_invalid), length( 0 ){}
        
        opcodes get_opcode() const { return (opcodes)opcode; }
        
        void init_header( opcodes op )
        {
            opcode = (byte)op;
            length = 0;
        }
    };

    class formatter : public Tmessage_formatter<header>
    {
    public:
        void init_header( opcodes op )
        {
             msg.get_header().init_header( op );
             parameters.clear( msg );
        }
        void add_byte( byte v )              { parameters.add_byte(v); }
        void add_word( word v )              { parameters.add_word(v); }
        void add_dword( dword v )            { parameters.add_dword(v); }
        void add_string( const char* v )     { parameters.add_string(v); }
        void add_string( const string& v )   { parameters.add_string(v); }
        void load( const system_message& _sm )
        {
            sysipc::system_message& sm = const_cast<sysipc::system_message&>(_sm);
            //msg.init( sm.payload(), sm.capacity() );
            //parameters.load( sm.payload(), sm.payload_capacity() );
            init_header( (opcodes) sm.payload()[0] );
            parameters.load( sm );
        }
    };

};

//----------------------------------------------------------------------------------------------------------------------
inline ostream& operator<<( ostream& strm, const Tmessage<msg_sp::header>& msg )
{
    for ( uint i = 0; i < msg.payload_size(); i++ )
        strm << hex << setw(2) << right << setfill('0') << (int)msg.payload()[i] << " ";
    strm << endl;
    return strm;
}


}
#endif

