#ifdef WIN32
#include <windows.h>
#endif
#include "sysipc_event_log.h" 
#include "sysipc_message_queue.h"
#ifdef WIN32
    #define LOG_WARNING 0
    #define syslog(a, ...) printf(__VA_ARGS__)
#else
    #include "syslog.h"
#endif

#ifdef WIN32
  #define QDIR "\\\\.\\pipe\\"
  #define QNAME_LG    QDIR "input_queue_lifeguard"
  #define QNAME_BMC   QDIR "input_queue_bmc"
  #define QNAME_SP    QDIR "input_queue_service_processor"
  #define QNAME_FM    QDIR "input_queue_flash_manager"
  #define QNAME_CM    QDIR "input_queue_config_manager"
  #define QNAME_CMR   QDIR "input_queue_config_manager"
  #define QNAME_TOOLS QDIR "input_queue_tools"
  #define QNAME_TKLM  QDIR "input_queue_tklm"
  #define QNAME_DP    QDIR "input_queue_dp"
  #define QNAME_PLDM    QDIR "input_queue_pldm"
#else
  #define QDIR "/lib/modules"
  #define QNAME_LG      QDIR
  #define QNAME_BMC     QDIR
  #define QNAME_SP      QDIR
  #define QNAME_FM      QDIR
  #define QNAME_CM      QDIR
  #define QNAME_CMR     QDIR
  #define QNAME_TOOLS   QDIR
  #define QNAME_TKLM    QDIR
  #define QNAME_DP      QDIR
  #define QNAME_PLDM    QDIR
#endif

namespace sysipc
{

//-------------------------------------------------------------------------------------------------------------
void system_message_queue_server::protected_run()
{
    while ( run_flag && listen()  )
    {
        // read from input queue
		if ( wait_message( false ) ) {
#ifdef WIN32
			sysipc::thread::sleep(1);
#endif
            dispatch();
		}
        else
        {
            syslog(LOG_WARNING, "MQ:Invalid message received\n");
            break;
        }
    }
}

//-------------------------------------------------------------------------------------------------------------
void system_message_queue_server::dispatch()
{
    handler_iter    i = find( handlers.begin(), handlers.end(), handler_entry( msg.get_header().mtype ) );
    if ( i != handlers.end() )
    {
        system_message_handler* hdlr = i->handler;
        hdlr->on_message( msg );
    }
    else
        system_message_queue_server::on_message( msg );
}

//-------------------------------------------------------------------------------------------------------------
void system_message_queue_server::on_message( const system_message& msg )
{
    syslog(LOG_WARNING, "MQ: Unhandled message received\n" );
}

//-------------------------------------------------------------------------------------------------------------
void system_message_queue_server::add_handler( long mtype, system_message_handler* hdlr )
{
    handler_entry   entry( mtype, hdlr );
    if ( handlers.end() == find( handlers.begin(), handlers.end(), entry ) )
         handlers.push_back( entry );
}

//-------------------------------------------------------------------------------------------------------------
void blocking_system_message_queue_server::run()
{
    if ( listen() )
    {
        if ( is_ok()  ) 
        {
            // read from input queue
            if ( wait_message( true ) )
                dispatch();
            else
                syslog(LOG_WARNING, "MQ: Invalid message received\n" );
        }
    }

    if ( !is_ok() )
        syslog(LOG_WARNING, "MQ: Receive failed, error = %d", (unsigned) error_code() );
}

    

//-------------------------------------------------------------------------------------------------------------
const char* system_message_queue::name( uint8 qid )
{
   const char* rslt = 0;
    switch ( qid )
    {
        case QUEUE_ID_BMC:
            rslt = QNAME_BMC;
            break;

        case QUEUE_ID_SP:
            rslt = QNAME_SP;
            break;

        case QUEUE_ID_FM:
            rslt = QNAME_FM;
            break;

        case QUEUE_ID_CM:
            rslt = QNAME_CM;
            break;

        case QUEUE_ID_CMR:
            rslt = QNAME_CMR;
            break;

        case QUEUE_ID_LG:
            rslt = QNAME_LG;
            break;

        case QUEUE_ID_TOOLS:
            rslt = QNAME_TOOLS;
            break;

        case QUEUE_ID_TKLM:
            rslt = QNAME_TKLM;
            break;

        case QUEUE_ID_DP:
            rslt = QNAME_DP;
            break;

        case QUEUE_ID_PLDM:
            rslt = QNAME_PLDM;
            break;

        default:
            rslt = 0;
    }
    return rslt;
}

//-------------------------------------------------------------------------------------------------------------
system_message_queues::system_message_queues( uint8 client_id ) : _client_id( client_id )
{
    // these must be pushed in order of ids so that ids can index the array
    system_message_queue_client lg_client( QUEUE_ID_LG, _client_id );
    clients.push_back( lg_client );

    system_message_queue_client bmc_client( QUEUE_ID_BMC, _client_id );
    clients.push_back( bmc_client );

    system_message_queue_client sp_client( QUEUE_ID_SP, _client_id );
    clients.push_back( sp_client );

    system_message_queue_client fm_client( QUEUE_ID_FM, _client_id );
    clients.push_back( fm_client );

    system_message_queue_client cm_client( QUEUE_ID_CM, _client_id );
    clients.push_back( cm_client );

    system_message_queue_client tools_client( QUEUE_ID_TOOLS, _client_id );
    clients.push_back( tools_client );

    system_message_queue_client cmr_client( QUEUE_ID_CMR, _client_id );
    clients.push_back( cmr_client );

    system_message_queue_client tklm_client( QUEUE_ID_TKLM, _client_id );
    clients.push_back( tklm_client );

    system_message_queue_client dp_client( QUEUE_ID_DP, _client_id );
    clients.push_back( dp_client );

    system_message_queue_client pldm_client( QUEUE_ID_PLDM, _client_id );
    clients.push_back( pldm_client );
}

//-------------------------------------------------------------------------------------------------------------
bool system_message_queues::send( system_message& msg )
{
    if ( msg.get_header().receiver < clients.size() )
    {
        msg.get_header().sender =  _client_id;
        return clients[ msg.get_header().receiver ].send( msg );
    }
    return false;
}

//----------------------------------------------------------------------------------------------------------------------
int system_message_transport::send( transport::receiver& rcvr, byte* data, byte data_length )
{
    request_message.set_payload( data, data_length );
    if ( request_queue.send( request_message ) )
    {
        cout.flush();
        response_queue.run();
        cout.flush();
        if ( response_queue.is_ok() && ( !response_queue.is_timeout() ) )
        {
            rcvr.receive( response_message.payload(), response_message.payload_size() );
            return 0;
        }
        else
        {
            syslog(LOG_WARNING, "MQ:system_message_transport receive error, %d\n", (int)response_queue.error_code());
            return -1;
        }
    }
    else
    {
        syslog(LOG_WARNING, "MQ:system_mesae_transport send error,%d\n", (int)request_queue.error_code());
        return -1;
    }
}

//----------------------------------------------------------------------------------------------------------------------
int system_message_transport_with_sequencing::send( transport::receiver& rcvr, byte* data, byte data_length )
{
    int rtn = -1;
    last_sent_seq_number++;
    request_message.get_header().seq_number = last_sent_seq_number;
    request_message.set_payload( data, data_length );
    if ( request_queue.send( request_message ) )
    {

        while ( true  )
        {
            response_queue.run();

            // if something went wrong break out
            if ( ( !response_queue.is_ok() ) || response_queue.is_timeout() )
                break;

            // Is this a response to this message??? If not, throw it away.
            if ( response_message.get_header().seq_number == last_sent_seq_number )
            {
                rcvr.receive( response_message.payload(), response_message.payload_size() );
                rtn = 0;
                break;
            }
        }
    }
    return rtn;
}


//----------------------------------------------------------------------------------------------------------------------
void system_message_transport::on_message( const system_message& msg )
{
    msg.copy_from( response_buffer, sizeof( response_buffer )  );
}




#ifdef WIN32
//*************************************************************************************************************
//
//    Windows implementations
//
//*************************************************************************************************************

//-------------------------------------------------------------------------------------------------------------
system_message_queue::system_message_queue( uint8 sid, int param ) :
        handle( INVALID_HANDLE_VALUE ), _server_id( sid ), ok( false ), _error_code( 0 ), _param( param )
{}

//-------------------------------------------------------------------------------------------------------------
system_message_queue::~system_message_queue()
{
    // really nothing to do here.
}

//-------------------------------------------------------------------------------------------------------------
int system_message_queue::make_key( uint8 qid )
{
    return qid;
}
 
//-------------------------------------------------------------------------------------------------------------
system_message_queue_server::system_message_queue_server( uint8 sid, int param ) :
    system_message_queue( sid, param ), msg( buffer, sizeof( buffer ) )
{ 
    run_flag = false;
    timeout = false;
}

//-------------------------------------------------------------------------------------------------------------
void system_message_queue_server::remove()
{
    // may want to destroy the pipe
}

//-------------------------------------------------------------------------------------------------------------
system_message_queue_server::~system_message_queue_server()
{
    //if ( is_ok() )
    //    remove();
}


//-------------------------------------------------------------------------------------------------------------
uint32 system_message_queue_server::receive( byte* buffer, uint size, bool no_wait )
{
    uint32 num_bytes_read = 0;
    uint32 num_bytes_written = 0;
    byte dummy[ 1 ];

    if ( is_ok() )
    {
        if ( 0 == ReadFile( handle, buffer, size, &num_bytes_read, 0 ) )
        {
            ok = false;
            _error_code = GetLastError();
        }
        WriteFile( handle, dummy, 1, &num_bytes_written, 0 );
    }

    return num_bytes_read;
}

//-------------------------------------------------------------------------------------------------------------
bool system_message_queue_server::listen()
{
    char* nm = const_cast<char*>( name( server_id() ) );
    if ( nm )
    {
        handle = CreateNamedPipe( nm, PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE, PIPE_UNLIMITED_INSTANCES, 4096, 4096, 0, 0 ); 
        ok = handle != INVALID_HANDLE_VALUE;
        if ( !ok )
            _error_code = GetLastError();
    }

    // Wait for the client to connect
    // If ConnectNamedPipe returns 0, everything ok. If non-zero, only allowed error is ERROR_PIPE_CONNECTED
    if ( is_ok() && ( 0 == ConnectNamedPipe( handle, 0 ) ) )
    {
        if ( GetLastError() != ERROR_PIPE_CONNECTED )
        {
            _error_code = GetLastError();
            ok = false;
        }
    }
    return is_ok();
}

//-------------------------------------------------------------------------------------------------------------
bool system_message_queue_client::send( const system_message& msg )
{
    if ( connect() )
    {
        uint32 num_bytes_read = 0;
        byte buffer[ 1 ];
        ok = true;
        char* nm = const_cast<char*>( name( server_id() ) );
        
        uint retries = 1200; // try for 10 minutes.. we might be debugging and this is windows....
        ok = 0 != CallNamedPipe( nm, (void*)msg.message(), msg.size(), buffer, 1, &num_bytes_read, 0 );
        while ( !ok )
        {
            _error_code = GetLastError();
            if ( ( _error_code != ERROR_PIPE_BUSY ) || ( retries-- == 0 ) )
                break;
			sysipc::thread::sleep(500); // try every half second
            ok = 0 != CallNamedPipe( nm, (void*)msg.message(), msg.size(), buffer, 1, &num_bytes_read, 100 );
        }
    }
    return is_ok();
}


//-------------------------------------------------------------------------------------------------------------
bool system_message_queue_client::connect()
{
    ok = true;
    _error_code = 0;
    return is_ok();
}

#else
//*************************************************************************************************************
//
//    Linux implementations
//
//*************************************************************************************************************
#include <sys/ipc.h>
#include <sys/msg.h>

//-------------------------------------------------------------------------------------------------------------
system_message_queue::system_message_queue( uint8 sid, int param ) :
        handle( -1 ), _server_id( sid ), ok( false ), _error_code( 0 ), _param( param ),
        key( make_key( sid ) )
{}

//-------------------------------------------------------------------------------------------------------------
system_message_queue::~system_message_queue()
{
    // really nothing to do here.
}

//-------------------------------------------------------------------------------------------------------------
QKEY system_message_queue::make_key( uint8 qid )
{
    const char* nm = name( qid );

    key_t k = BAD_KEY;

    // ftok is pretty lame. Must have an existing file etc (something with an inode) for first parameter
    // second parameter must be integer in range 1-255. Since qids are 0,1,... we use 'Q' + qid.
    if ( nm )
    {
        k = ftok( nm, qid + 'Q' );
        if ( k == BAD_KEY )
            syslog(LOG_WARNING, "MQ:make key error, error = %s\n", strerror( errno ) );
    }
    return k;
}



//-------------------------------------------------------------------------------------------------------------
system_message_queue_server::system_message_queue_server( uint8 sid, int param ) :
    system_message_queue( sid, param ), msg( buffer, sizeof( buffer ) )

{
    run_flag = false;
    bool is_purge_needed = true;
    
    if ( key != BAD_KEY )
    {
        if ( sid == QUEUE_ID_SP && is_exsit() )
            is_purge_needed = false;
    
        // create semphore for sending msg from bmc_app and flash_manager processes.
        semhandle = semget( key, 1, IPC_CREAT | 0666 );

        // Create the queue if it doesn't already exist. Not exclusive because if process crashes this call will fail
        // Next time around.
        // If the queue already exists, may want to flush it. That may have to be part of permissions.
        // The server only needs read permission on this queue. Clients need write permission.
        // The permissions are <??><User><Owner><Group>, the numbers of interest 2-write, 4-read, 6-R/W
        // If MSG_NOERROR is specified, then the message text will be truncated (and the  truncated part will be lost); if MSG_NOERROR is not specified, then the message isn't removed from the queue and the system call fails returning -1 with errno set to E2BIG.
        handle = msgget( key, IPC_CREAT | 0666 );
        ok = (handle >= 0) && (semhandle >= 0);
        if ( handle == -1 || semhandle == -1 )
        {
            _error_code = errno;
            syslog(LOG_WARNING, "MQ:system_message_queue_server constructor error, error = %s\n", strerror( errno ) );
        }
        else
        {
            // set to one resouce available on this semahore. 
            if ( semctl(semhandle, 0, SETVAL, 1) == -1 )
            {
                _error_code = errno;
                syslog(LOG_WARNING, "MQ:system_message_queue_server semctl error, error = %s\n", strerror( errno ) );
            }

            if ( is_purge_needed )
            {
                // purge the queue of any state requests
                msqid_ds    queue_info;
                int rtn = msgctl(  handle, IPC_STAT, &queue_info );
                while ( ( rtn != -1) && ( queue_info.msg_qnum > 0 ) )
                {
                    // loop till there are no messages in the queue
                    rtn = msgrcv( handle, buffer, sizeof(buffer), 0, IPC_NOWAIT | MSG_NOERROR );
                    if ( rtn >= 0 )
                        rtn = msgctl(  handle, IPC_STAT, &queue_info );
                }
                if ( rtn == -1 )
                {
                    // failed to purge the queue. declare this queue dead
                    handle = -1;
                    _error_code = errno;
                    ok = false;
                }
            }
        }
    }
}

//-------------------------------------------------------------------------------------------------------------
system_message_queue_server::~system_message_queue_server()
{
    if ( is_ok() )
        remove();
}

//-------------------------------------------------------------------------------------------------------------
void system_message_queue_server::remove()
{
    //if ( handle >= 0 )
    //{
    //    msqid_ds    queue_info;
    //    msgctl(  handle, IPC_RMID, &queue_info );
    //    handle = -1;
    //    ok = false;
    //}
}

//-------------------------------------------------------------------------------------------------------------
bool system_message_queue_server::listen()
{
    // There is really nothing to do here as there is no notion of connections with system V queues
    return is_ok();
}

//-------------------------------------------------------------------------------------------------------------
uint32 system_message_queue_server::receive( byte* buffer, uint size, bool no_wait )
{
    ssize_t num_bytes_read = 0;
    memset(buffer, 0, size);

    if ( is_ok() )
    {
        // next to last parameter 0 means we want first message on queue
        // last parameter means block
        // purge the queue of any state requests

        /* Sean - unused variables 
            msqid_ds    queue_info;
            int rtn = msgctl(  handle, IPC_STAT, &queue_info );
        */
        // If MSG_NOERROR is specified, then the message text will be truncated (and the  truncated part will be lost); if MSG_NOERROR is not specified, then the message isn't removed from the queue and the system call fails returning -1 with errno set to E2BIG.
        timeout = false;
        if ( no_wait )
        {
            retry_timer timer( 20 );
            while ( timer.wait() )
            {
                num_bytes_read = msgrcv( handle, buffer, size, 0, IPC_NOWAIT | MSG_NOERROR);
                if ( ( num_bytes_read > 0 ) || ( ( num_bytes_read == -1 ) && ( errno != ENOMSG ) ) )
                    break;
                num_bytes_read = 0;
            }
            timeout = num_bytes_read == 0;
            if( timeout )
                syslog(LOG_WARNING, "MQ: system_message_queue_server: msgrcv timeout, handle=%d,error = %s\n", handle, strerror( errno ) );
        }
        else
            num_bytes_read = msgrcv( handle, buffer, size, 0, MSG_NOERROR );

        if ( num_bytes_read == -1 )
        {
            ok = false;
            _error_code = errno;
            num_bytes_read = 0;

            if ( errno == EINTR )
                remove();
            else
                syslog(LOG_WARNING, "MQ: system_message_queue_server receive, error = %s\n", strerror( errno ) );
        }
    }
    return num_bytes_read;
}

//-------------------------------------------------------------------------------------------------------------
bool system_message_queue_client::send( const system_message& msg )
{
    struct sembuf semoplock, semopunlock;

    if ( connect() ) 
    {
        // If an operation specifies SEM_UNDO, it will be automatically undone when the process terminates.
        init_semops(&semoplock, 0, -1, IPC_NOWAIT | SEM_UNDO);
        init_semops(&semopunlock, 0, 1, IPC_NOWAIT | SEM_UNDO);

        // We may be connected, but the server could be dead. No way to really know this other than his 
        // queue is backing up. So we will declare him busy if his queue gets bigger that 64 ( kinda arbitrary ).
        // A better way may be to have server remove the queue. Wil llook into this.
        msqid_ds    queue_info;
        int rtn = msgctl(  handle, IPC_STAT, &queue_info );
        if ( rtn != -1 )
        {
            // Specifying 0 in last parameter means we want to wait.
            // If queue full, don't set any error codes, just return false
            if ( queue_info.msg_qnum < 64 )
            {
                int lockrc = -1, unlockrc = -1;
                int retry = 0;

                if ( queue_info.msg_qnum > 48 )
                {
                    syslog(LOG_WARNING, "MQ: system_message_queue_client send, msg_qnum =%d, client=%d, server=%d, payload[0]=%d\n",
                        (int)queue_info.msg_qnum, _client_id, _server_id, (int)msg.payload()[0]);
                }

                lockrc = semop(semhandle, &semoplock, 1);
                rtn = msgsnd( handle, msg.message(), msg.size(), 0 );
                // syslog(LOG_WARNING, "MQ: system_message_queue_client send, rtn=%d, size=%d, handle=%x\n", rtn, (int) msg.size(), (unsigned) handle );
                while ( lockrc == 0 && unlockrc != 0 && (retry++ < 5) )
                {
                    unlockrc = semop(semhandle, &semopunlock, 1);
                }
                if ( retry >= 5 )
                {
                    syslog(LOG_WARNING, "MQ: system_message_queue_client send, unlock =%s\n", strerror( errno ) );
                }
            }
            else
            {
               syslog(LOG_ERR, "ERROR: MQ: system_message_queue_client send, msg_qnum =%d, client=%d, server=%d, payload[0]=%d\n",
                    (int)queue_info.msg_qnum, _client_id, _server_id, (int)msg.payload()[0]);
            }
        }
        if ( rtn == -1 )
        {
            // don't declare 
            ok = false;
            _error_code = errno;
            syslog(LOG_WARNING, "MQ: system_message_queue_client send, error =%s\n", strerror( errno ) );
        }
    }
    return is_ok();
}

//-------------------------------------------------------------------------------------------------------------
long system_message_queue_client::num_messages()
{
    long msg_qnum = -1; 

    if ( connect() ) 
    {
        msqid_ds    queue_info;

        int rtn = msgctl(  handle, IPC_STAT, &queue_info );
        if ( rtn != -1 )
            msg_qnum = (long) queue_info.msg_qnum;
        else
            syslog(LOG_WARNING, "MQ:system_message_queue_server num_messages error, error = %s\n", strerror( errno ) );
    }

    return msg_qnum;
}

//-------------------------------------------------------------------------------------------------------------
bool system_message_queue_client::connect()
{
    // don't try to connect if we are already connected
    if ( !is_ok() )
    {
        if ( key != BAD_KEY )
        {
            semhandle = semget(key, 1, 0666);

            // Clients are connecting to a server queue so they need write access. On the other hand they don't need
            // read access. If the queue doesn't exist we want this to fail.
            // The permissions are <??><User><Owner><Group>, the numbers of interest 2-write, 4-read, 6-R/W
            handle = msgget( key, 0666 );
            ok = (handle >= 0) && (semhandle >= 0);
            if ( handle == -1 || semhandle == -1 )
                _error_code = errno;
        }
    }
    return is_ok();
}
int system_message_queue_client::init_semops(struct sembuf * semops, unsigned short semnum, short semop, short semflg)
{
    if (semops == NULL)
    {
        syslog(LOG_WARNING, "MQ:error, parameter semops is null pointer.\n");
        return -1;
    }
    semops->sem_num = semnum;
    semops->sem_op = semop;
    semops->sem_flg = semflg;
    return 0;
}

#endif
} // end namespace





