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
#include "sysipc_application_control.h"

namespace sysipc
{
void application_control::handler::on_message( const system_message& msg )
{
    request_message.init( (byte*)msg.payload(), msg.payload_size() );
    if( parameters.load( request_message ) )
    {
        application_control::opcodes op = ( application_control::opcodes ) request_message.get_header().get_opcode();
        switch( op )
        {
            case op_set_log_flags:
                if( parameters.num_params() == 2 )
                {
                    //byte sub_sys = byte_tlv( ( parameters[ 0 ] ) ).get_value();
                    //byte flags   = dword_tlv( ( parameters[ 1 ] ) ).get_value();
                    // what do I do here????
                }
                break;

            case op_set_log_level:
                if( parameters.num_params() == 2 )
                {
                    byte log    = byte_tlv( ( parameters[ 0 ] ) ).get_value();
                    byte lev  = byte_tlv( ( parameters[ 1 ] ) ).get_value();
                    if( log == 0 )
                        dbg_log.set_level( (event_log::levels) lev );
                    else
                        app_log.set_level( (event_log::levels) lev );
                }
                break;

            case op_set_stdio_log_output:
                if( parameters.num_params() == 2 )
                {
                    byte log    = byte_tlv( ( parameters[ 0 ] ) ).get_value();
                    bool enable = 0 != byte_tlv( ( parameters[ 1 ] ) ).get_value();
                    if( log == 0 )
                    {
                        if( enable )
                            dbg_log.enable_stdout();
                        else
                            dbg_log.disable_stdout();
                    } else
                    {
                        if( enable )
                            app_log.enable_stdout();
                        else
                            app_log.disable_stdout();
                    }
                }
                break;

            case op_set_file_log_output:
                if( parameters.num_params() == 2 )
                {
                    byte log  = byte_tlv( ( parameters[ 0 ] ) ).get_value();
                    bool enable = 0 != byte_tlv( ( parameters[ 1 ] ) ).get_value();
                    if( log == 0 )
                    {
                        if( enable )
                            dbg_log.open_file();
                        else
                            dbg_log.close_file();
                    } else
                    {
                        if( enable )
                            app_log.open_file();
                        else
                            app_log.close_file();
                    }
                }
                break;

            case op_purge_log_file:
                if( parameters.num_params() == 2 )
                {
                    //byte    log    = byte_tlv( ( parameters[ 0 ] ) ).get_value();
                    //dword   lines  = dword_tlv( ( parameters[ 1 ] ) ).get_value();
                }
                break;

            case op_initiate_shut_down:
                if( app_control )
                    app_control->shutdown( true );

                break;

            case op_alert_system_state:
                if( app_control )
                {    
                    system_states ev  = (system_states) byte_tlv( ( parameters[ 0 ] ) ).get_value();
                    app_control->alert_system_state( ev );
                }
                break;

           case op_diagnostic:
                if ( parameters.num_params() > 0 )
                {
                    byte param = 0;
                    string_tlv diag_op( parameters[ 0 ] );
                    if ( parameters.num_params() > 1 )
                        param = ( byte_tlv( parameters[ 1 ] ) ).get_value();
                    app_control->diagnostic( diag_op.get_value(), param );
                }
                break;

            default:
                break;

        }
    }
}

const char* application_control::map( system_states state )
{
    switch ( state )
    {
        case 0: return "unknown"         ;
        case 1: return "power_not_good"  ;
        case 2: return "power_good"      ;
        case 3: return "os_post_complete";
        case 4: return "os_running"      ;
        case 5: return "os_not_running"  ;
        case 7: return "suspend_to_ram"  ;
        default: return "undefined";
    }

}
}
