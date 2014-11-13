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
#ifndef __SYSIPC_MESSAGE_QUEUE_H
#define __SYSIPC_MESSAGE_QUEUE_H

#include "sysipc.h"
#include "sysipc_messages.h"
#include "sysipc_threads.h"

/// \ingroup system_messages
/// \file bmc_system_message_queue.h
/// \brief Interprocess message queues

#ifdef WIN32
#define QHANDLE void*
#define QKEY int
#else
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <errno.h>
#define QKEY key_t
#define QHANDLE int
#endif

#define QUEUE_ID_LG         0
#define QUEUE_ID_BMC        1
#define QUEUE_ID_SP         2
#define QUEUE_ID_FM         3
#define QUEUE_ID_CM         4
#define QUEUE_ID_TOOLS      5
#define QUEUE_ID_CMR        6
#define QUEUE_ID_TKLM       7
#define QUEUE_ID_DP         8
#define QUEUE_ID_PLDM       9
#define QUEUE_ID_NONE       255

#define BAD_KEY  -1

namespace sysipc
{

/// \ingroup system_messages
/// \brief system_message_handler
class system_message_handler
{
public:
    virtual void on_message( const system_message& msg ) = 0 ;
};


/// \ingroup system_messages
/// \brief system_message_queue
class system_message_queue
{
protected:
    QHANDLE                         handle;
    uint8                           _server_id;
    bool                            ok;
    uint32                          _error_code;
    int                             _param;
    QKEY                            key;
    QHANDLE                         semhandle;

    system_message_queue( uint8 sid, int param );
    system_message_queue() : ok( false ){}
public:
    virtual ~system_message_queue() = 0;
    static const char* name( uint8 qid );
    static QKEY make_key( uint8 qid );
    uint8 server_id() const { return _server_id; }
    bool is_ok() const { return ok; }
    uint32 error_code() const { return _error_code; }
};

inline ostream& operator<<( ostream& strm, const system_message& msg )
{
    strm << "Message: type(" << msg.get_header().mtype << "), sender(" << (uint)msg.get_header().sender << "), receiver(" << (uint)msg.get_header().receiver << "), size(" << (uint)msg.payload_size() << ")" << endl;
    return strm;
}


//------------------------------------------------------------------------------------------
/// \brief system message_queue_server
class system_message_queue_server : public system_message_queue, public system_message_handler
{
protected:
    struct handler_entry
    {
        long                    mtype;
        system_message_handler* handler;
        handler_entry( long mt = 0, system_message_handler* h = 0 ) : mtype( mt ), handler( h ){}
        bool operator==( const handler_entry& e ) const { return mtype == e.mtype; }
    };
    /// \brief Incoming message buffer
    byte                        buffer[ MAX_SYSTEM_MESSAGE_SIZE ];

    bool run_flag;

    /// \brief Incoming message buffer
    system_message              msg;

    /// \brief Clients that want to be notified when a message arrives
    vector<handler_entry>       handlers;

    bool timeout;


    typedef vector<handler_entry>::iterator handler_iter;

    /// \breif dispatches messages to handlers
    void dispatch();

    /// \brief Operating specifc method to actually get the data
    uint32 receive( byte* buffer, uint size, bool no_wait );

    /// \brief Derived run methods MUST call this method to wait on reveive messages
    /// \return true is message ok, false otherwise
    bool wait_message( bool no_wait )
    {
        uint32 bytes_received = receive( msg.message(), msg.capacity(), no_wait );

        // Check for incomplete header, fragmented payload, screwed up parameter list
        return ( bytes_received >= msg.header_size() ) && ( !msg.is_overrun() ) && (msg.size() == bytes_received);
    }
    bool listen();

    void protected_run();

    void remove();

public:
            system_message_queue_server( uint8 server_id, int param );
            ~system_message_queue_server();
    void    on_message( const system_message& msg );
    void    add_handler( long mtype, system_message_handler* handler );
    void    stop(){ run_flag = false; }

//This is a fix to build this project in windows. mssget is a Linux function
#ifndef WIN32
    bool    is_exsit() { return (BAD_KEY != key) && (msgget(key, 0666) >=0); }
#endif

    /// \brief Derived classes can supply their own run method. They must use base class wait_message
    virtual void run(){ run_flag = true; protected_run(); }
};

//------------------------------------------------------------------------------------------
/// \brief threaded_system message_queue_server
class threaded_system_message_queue_server : public system_message_queue_server
{
    thread*  service_thread;

public:
    static void service_thread_proc( threaded_system_message_queue_server* me )
    {
        me->run_flag = true;
        me->protected_run();
    }

    threaded_system_message_queue_server( uint8 in_server_id, int param ) :
        system_message_queue_server( in_server_id, param )
    {
        service_thread = Tthread<threaded_system_message_queue_server>::create( "bmc system message queue thread", service_thread_proc, this );
    }

    virtual void run(){ service_thread->run(); }
};

//------------------------------------------------------------------------------------------
/// \brief threaded_system message_queue_server
class blocking_system_message_queue_server : public system_message_queue_server
{
public:
    blocking_system_message_queue_server( uint8 in_server_id, int param ) :
        system_message_queue_server( in_server_id, param )
    {}
    virtual void run();
    bool is_timeout(){ return timeout; }
};

//------------------------------------------------------------------------------------------
/// \brief system message_queue_client
class system_message_queue_client : public system_message_queue
{
    uint8 _client_id;
public:
    system_message_queue_client( uint8 in_server_id, uint8 in_client_id, int param = 0 ) :
        system_message_queue( in_server_id, param ), _client_id( in_client_id ){}

    system_message_queue_client(){}

    bool connect();
    bool send( const system_message& message );
    uint8 client_id() const { return _client_id; }
    int init_semops(struct sembuf * semops, unsigned short semnum, short semop, short semflg);
    long num_messages();
};

//------------------------------------------------------------------------------------------
class system_message_queues
{
    uint8 _client_id;
    vector<system_message_queue_client> clients;

public:
    system_message_queues( uint8 client_id );
    bool send( system_message& msg );
};

//----------------------------------------------------------------------------------------------------------------------
/// \brief This class abstracts the transpot layer for FM Clients.
class transport
{
public:
    /// \brief Transport layers use this abstract interface to pass receive messages to FM Clients.
    class receiver 
    {
    public:
        virtual void receive( byte* data, byte data_length ) = 0;
    };

    enum errors
    {
        no_error    = 0,
        busy        = 1,
    };
    /// \brief Clients call this method to pass request messages to transport layer.
    /// \return 0 if data is valid, -1 if not.
    virtual int send( receiver& rcvr, byte* data, byte data_length ) = 0;
};
 
//----------------------------------------------------------------------------------------------------------------------
/// \brief Use system messages as flash manager transport
class system_message_transport : public transport, public system_message_handler
{
protected:
    blocking_system_message_queue_server    response_queue;
    system_message_queue_client             request_queue;
	byte                                            request_buffer[ sysipc::MAX_SYSTEM_MESSAGE_SIZE ];
	byte                                            response_buffer[ sysipc::MAX_SYSTEM_MESSAGE_SIZE ];
	system_message                                  request_message;
	system_message                                  response_message;

public:
    system_message_transport( byte this_q, byte remote_q ) :
        response_queue( this_q, 0 ), request_queue( remote_q , 0), 
        response_message( response_buffer, sizeof( response_buffer ) )
    {
        request_message.init( request_buffer, sizeof( request_buffer ) );
        response_message.init( response_buffer, sizeof( response_buffer ) );
        request_message.get_header().init_request( MESSAGE_FM, this_q, remote_q ); 
        response_queue.add_handler( MESSAGE_FM, this );
    }
    virtual ~system_message_transport(){};

    // from transport
	int send( transport::receiver& rcvr, byte* data, byte data_length );

    // from system message handler
    void on_message( const system_message& msg );
};

//----------------------------------------------------------------------------------------------------------------------
/// \brief Use system messages as flash manager transport
class system_message_transport_with_sequencing : public system_message_transport
{
    byte    last_sent_seq_number;

public:
    system_message_transport_with_sequencing( byte this_q, byte remote_q ) :
        system_message_transport( this_q, remote_q )
    {
        last_sent_seq_number = 0;
    }

    // from transport
	int send( transport::receiver& rcvr, byte* data, byte data_length );
};

} // end namespace
#endif

