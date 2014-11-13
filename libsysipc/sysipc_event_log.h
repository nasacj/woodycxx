/******************************************************************************
 * 
 *  Licensed Materials - Property of IBM.
 * 
 *  (C) Copyright IBM Corporation 2007-2008
 * 
 *  All Rights Reserved.
 * 
 *  US Government Users Restricted Rights -
 *  Use, duplication or disclosure restricted by
 *  GSA ADP Schedule Contract with IBM Corporation.
 * 
 *****************************************************************************/
#ifndef __SYSIPC_EVENT_LOG_H
#define __SYSIPC_EVENT_LOG_H

#include "sysipc.h"
#include "sysipc_threads.h"
#include "sysipc_event_log.h"
#include <fstream>
#include <sstream>

namespace sysipc
{

const uint DEBUG_ASSERTIONS     = 0x00000001;
const uint DEBUG_ERRORS         = 0x00000002;
const uint DEBUG_INFORMATION    = 0x00000004;

const uint DEBUG_LEVEL_0        = DEBUG_ASSERTIONS | DEBUG_ERRORS;
const uint DEBUG_LEVEL_5        = DEBUG_INFORMATION;

class streams;
//------------------------------------------------------------------------------------------
/// \ingroup sysipc
/// \brief event_log 
class event_log
{
    friend class application_log;
    friend class debug_log;
public:
    struct streams
    {
        bool                log_to_file;
        bool                log_to_stdout;
        bool                log_to_syslog;
        ostream*            file_strm;
        int                 facility;
        uint                mask;
        bool                purging;
        critical_section    crit_sec;
    public:
        streams() : log_to_file( false ), log_to_stdout( true ), log_to_syslog( false ), facility( -1 ), mask( 0xFFFFFFFF ){}
        void append( const char* p, const string& ent );
    };

    enum logs
    {
        app_log,
        dbg_log
    };

    enum levels
    {
        level_0 = 0,
        level_1,
        level_2,
        level_3,
        level_4,
        level_5,
        level_6,
        level_7,
        num_levels
    };
    struct prefix
    {
        const char* _prefix;
        prefix( const char* p ) : _prefix( p ){}
    };



protected:
    string      _filename;
    ofstream    _file;
    streams     _streams;
    levels      _level;
    uint        masks[ num_levels ];



public:
    event_log( const char* nm );
    event_log();
    ~event_log(){ close_file(); }
    void open_file();
    void close_file();
    void open_syslog(const char* ident, int localn);
    void close_syslog();
    void write_mask( levels lev, uint m );
    void set_mask( levels lev, uint m ){ write_mask( lev, masks[ lev ] | m ); }
    void clear_mask( levels lev, uint m ){ write_mask( lev, masks[ lev ] & ~m ); }
    void enable_stdout(){ _streams.log_to_stdout = true; }
    void disable_stdout(){ _streams.log_to_stdout = false; }
    bool is_enabled( uint mask ) const { return 0 != ( _streams.mask & mask ); }
    uint get_mask() const { return _streams.mask; }
    int  get_facility() const { return _streams.facility; }
    levels get_level() const { return _level; }
    void set_level( levels lev )
    {
        if ( lev < num_levels )
            _level = lev;
        else
           _level = (event_log::levels) ( num_levels - 1 );

        _streams.mask = masks[ _level ];
    }
};

inline ostream& operator<<( ostream& strm, const event_log::prefix& p )
{ 
    strm << "<" << p._prefix << ">";
    return strm;
}


//------------------------------------------------------------------------------------------
class application_log : public event_log
{
    static event_log*  app_streams;
public:
    static bool is_enabled( uint m ){ return app_streams && app_streams->is_enabled( m );  }
    application_log( const char* nm ) : event_log( nm )
    {
        if ( !app_streams )
            app_streams = this;

    }
    ~application_log()
    {
        if ( app_streams == this )
            app_streams = 0;
    }
    static void append( const char* p, const string& str )
    { 
        if ( app_streams ) 
            app_streams->_streams.append( p, str );
    }
    static void init( event_log* log ){ app_streams = log; }
    static event_log* get_log() { return app_streams; }
    static levels get_level()
    {
        if (app_streams)
            return app_streams->get_level();

        return num_levels;
    }
    static uint get_mask()
    {
        if (app_streams)
            return app_streams->get_mask();

        return 0;
    }
    static int get_facility()
    {
        if (app_streams)
            return app_streams->get_facility();

        return -1;
    }
};

//------------------------------------------------------------------------------------------
class debug_log : public event_log
{
    static event_log*  debug_streams;
public:
    static bool is_enabled( uint m ){ return debug_streams && debug_streams->is_enabled( m );  }
    debug_log( const char* nm ) : event_log( nm )
    {
        if ( !debug_streams )
        {
            debug_streams = this;
            write_mask( level_0, DEBUG_LEVEL_0 );
            write_mask( level_5, DEBUG_LEVEL_5 );
        }
    }
    ~debug_log()
    {
        if ( debug_streams == this )
            debug_streams = 0;
    }
    static void append( const char* p, const string& str )
    { 
        if ( debug_streams ) 
            debug_streams->_streams.append( p, str );
    }
    static void init( event_log* log ){ debug_streams = log; }
    static event_log* get_log() { return debug_streams; }
};


#define LOG_ENTRY(prefix,m,message) do { \
    if ( sysipc::application_log::is_enabled( m ) ) \
    { \
        ostringstream strm; \
        strm << message; \
        sysipc::application_log::append( prefix, strm.str() ); \
    } \
} while ( 0 )


#define DEBUG_ENTRY(prefix,m,message) do { \
    if ( sysipc::debug_log::is_enabled( m ) ) \
    { \
        ostringstream strm; \
        strm << message; \
        sysipc::debug_log::append( prefix, strm.str() ); \
    } \
} while ( 0 )

#define DEBUG_ERROR(prefix,message) DEBUG_ENTRY(prefix,sysipc::DEBUG_ERRORS,message)
#define DEBUG_INFO(prefix,message) DEBUG_ENTRY(prefix,sysipc::DEBUG_INFORMATION,message)



} // end namespace

#endif
