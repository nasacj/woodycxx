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
#ifndef __SYSIPC_APPLICATION_CONTROL_H
#define __SYSIPC_APPLICATION_CONTROL_H

#include "sysipc_message_queue.h"
#include "sysipc_event_log.h"
#include <iomanip>

/// \defgroup app_control Application Control Messages
/// 
/// \ingroup app_control
/// \file bmc_application_control.h
/// \brief Application Control Messages

//------------------------------------------------------------------------------------------------------------------
/// \defgroup applicaton_control sysipc
/// \file sysipc._application_control.h
/// \brief Messages
///
/// Application control messages
/// 
//------------------------------------------------------------------------------------------------------------------
namespace sysipc
{

//------------------------------------------------------------------------------------------------------------------
/// \ingroup applicaton_control
/// \brief Enclosing class for all apllication control classes
class application_control
{
public:
    enum opcodes
    {
        op_ping                    = 0x00,
        op_alert_state             = 0x01,
        op_alert_data_change       = 0x10,
        op_set_log_level           = 0x03,
        op_set_log_flags           = 0x08,
        op_bmc_log_cntrl           = 0x09,
        op_bmc_log_set             = 0x0A,
        op_set_stdio_log_output    = 0x04,
        op_set_file_log_output     = 0x05,
        op_purge_log_file          = 0x06,
        op_diagnostic              = 0x07,
        op_alert_system_state      = 0x11,
        op_send_alert              = 0x0b,
        op_dump_data               = 0x0c,
        op_initiate_shut_down      = 0x80,
        op_file_event              = 0x20,
        op_htc_event               = 0x21,
        op_invalid                 = 0xFF
    };
    
    enum dump_type
    {
        dump_led            = 0x10,
        dump_invalid        = 0xFF
    };
    
    enum htc_event_type
    {
        ssh_host_key_regenerated               = 0,
        bcr_pwr_allow                          = 1,
        bcr_pwr_disallow                       = 2,
        default_server_cert_generation_success = 3,
        default_server_cert_generation_abort   = 4,
        csr_generation_success                 = 5,
        csr_generation_abort                   = 6
    };

    enum states
    {
        critical_init   = 0,
        init            = 1,
        running         = 2,
        exit            = 14,
        critical_exit   = 15
    };

    enum system_states
    {
        unknown             = 0,
        power_not_good      = 1,
        power_good          = 2,
        os_post_complete    = 3,
        os_running          = 4,
        os_not_running      = 5,
        bios_post_complete  = 6,
        suspend_to_ram      = 7
    };
    static const char* map( system_states state );

    struct header
    {
       byte    opcode;
       byte    length;

       byte    payload_size() const { return length; }
       byte&   payload_size() { return length; }

        header() : 
            opcode(op_ping), length( 0 ){}

        opcodes     get_opcode() const { return (opcodes)opcode; }

        void init_header( opcodes op )
        {
            opcode = (byte)op;
            length = 0;
        }
    };
    typedef Tmessage<header>            message;
    typedef Tparameter_list<header>     parameter_list;

    class control_interface
    {
    public:
        virtual bool shutdown( bool is_critical ){ return false;};
        virtual void diagnostic( const string& diag_op, byte param ){}
        virtual void purge_log(  const string& diag_op, byte param ){}
        virtual void alert_system_state( system_states sys_state ){}
        virtual void alert_application_state( states app_state ){}
    };


    /// \brief Application control handler
    class handler : public system_message_handler
    {
        message                     request_message;
        Tparameter_list<header>     parameters;
        control_interface*          app_control;

    public:
        debug_log                   dbg_log;
        application_log             app_log;

        handler( const char* app_logname, const char* dbg_logname, control_interface* intf = 0 ) : 
            app_control( intf ), dbg_log( dbg_logname ), app_log( app_logname ){}
        void set_handler( control_interface* intf ){ app_control = intf; }
        virtual ~handler() {};
        virtual void on_message(const system_message& msg);
    };


    class message_formatter : public Tmessage_formatter<header>
    {
        void init_header( application_control::opcodes op )
        {
             msg.get_header().init_header( op );
             parameters.clear( msg );
        }

    public:
        /// \brief Notify others of current state
        /// \param current_state Current application state
        void alert_state( states current_state )
        {
            init_header( op_alert_state );
            parameters.add_byte( (byte) current_state );
            finish();
        }

        /// \brief Notify others of current state
        /// \param current_state Current application state
        void alert_system_state( system_states current_state )
        {
            init_header( op_alert_system_state  );
            parameters.add_byte( (byte) current_state );
            finish();
        }

        /// \brief Notify of application data change
        /// \param param param Reserved
        void alert_data_change( byte param )
        {
            init_header( op_alert_data_change );
            parameters.add_byte( param );
            finish();
        }

        /// \brief Notify of application data change
        /// \param param param Reserved
        void send_alert( byte alert, byte value )
        {
            init_header( op_send_alert );
            parameters.add_byte( alert );
            parameters.add_byte( value );
            finish();
        }

        /// \brief request application to dump data
        /// \param param param Reserved
        void send_dump_request( byte type )
        {
            init_header( op_dump_data );
            parameters.add_byte( type );
            finish();
        }

        /// \brief request application to send hitachi event
        /// \param param param Reserved
        void send_htc_event_request( byte param )
        {
            init_header( op_htc_event );
            parameters.add_byte( param );
            finish();
        }

        /// \brief Ping an application for it current state
        void ping()
        {
            init_header( op_ping);
            finish();
        }

        /// \brief Set logging level for application/debug logs
        /// \param log 0:debug log, 1:application log
        /// \param level, log level 0-7
        void set_log_level( byte log, sysipc::event_log::levels level )
        {
            init_header( op_set_log_level );
            parameters.add_byte( log );
            parameters.add_byte( (byte) level );
            finish();
        }

        /// \brief Set logging level for BMC application style logs
        /// \param sub_system (io, cpu, thermal, amm, etc...; see bmc_app[ bmc_log.h ] )
        /// \param flags, log mask
        void set_log_flags( byte sub_system, dword flags )
        {
            init_header( op_set_log_flags );
            parameters.add_byte( sub_system );
            parameters.add_dword( (dword) flags );
            finish();
        }
  /// \brief BMC log control
        /// \param comp ( warn/notice )
        /// \param op ( reset/start/stop )
        void bmc_log_cntrl( byte comp, byte op )
        {
            init_header( op_bmc_log_cntrl);
            parameters.add_byte( comp );
            parameters.add_byte( op );
            finish();
        }

        /// \brief BMC log threshold set
        /// \param comp ( warn/notice )
        /// \param threshold ( integer )
        void bmc_log_set( byte comp, dword threshold )
        {
            init_header( op_bmc_log_set);
            parameters.add_byte( comp );
            parameters.add_dword( (dword)threshold );
            finish();
        }


        /// \brief Enable/disable log output to stdio
        /// \param log 0:debug log, 1:application log
        /// \param 0: disable, 1: enable
        void set_stdio_log_output( byte log, byte enable )
        {
            init_header( op_set_stdio_log_output );
            parameters.add_byte( log );
            parameters.add_byte( (byte) enable );
            finish();
        }

        /// \brief Enable/disable log output to file
        /// \param log 0:debug log, 1:application log
        /// \param 0: disable, 1: enable
        void set_file_log_output( byte log, byte enable )
        {
            init_header( op_set_file_log_output );
            parameters.add_byte( log );
            parameters.add_byte( (byte) enable );
            finish();
        }

        /// \brief Purges lines from log file
        /// \param log 0:debug log, 1:application log
        /// \param num_lines 0:entire file, >0: number of lines
        void purge_log_file( byte log, dword num_lines )
        {
            init_header( op_set_file_log_output );
            parameters.add_byte( log );
            parameters.add_dword( num_lines );
            finish();
        }

        /// \brief Initiate shutdown
        /// \param type 0:non-critical, 1:critical
        void initiate_shutdown( byte type )
        {
            init_header( op_initiate_shut_down );
            parameters.add_byte( type );
            finish();
        }

        /// \brief Send a diagnostic command
        /// \param op       Application specific operation.
        /// \param param    parameter
        void diagnostic(  const char* op, byte param )
        {
            init_header( op_diagnostic );
            parameters.add_string( op );
            parameters.add_byte( param );
            finish();
        }

        /// \brief file vent
        /// \param log 0:file name
        /// \param 0: file operation
        void set_file_event( byte file_index, dword oper)
        {
            init_header( op_file_event );
            parameters.add_byte( file_index );
            parameters.add_dword(  oper );
            finish();
        }

    };
};

//----------------------------------------------------------------------------------------------------------------------
inline ostream& operator<<( ostream& strm, const application_control::message& msg )
{
    for ( uint i = 0; i < msg.size(); i++ )
        strm << hex << setw(2) << right << setfill('0') << (int)msg.message()[i] << " ";
    strm << endl;
    return strm;
}


//----------------------------------------------------------------------------------------------------------------------
/// \ingroup app_control
/// \brief Application Control Messages

}
#endif

