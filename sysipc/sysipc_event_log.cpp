/*
  Copyright (c) 2014-2015 by NASa Qian <nasacj@nasacj.net>
  This file is part of the woodycxx library.

  This software is distributed under BSD 3-Clause License.
  The full license agreement can be found in the LICENSE file.

  This software is distributed without any warranty.
*/

#include "sysipc_event_log.h"

#ifndef LOG_LOCAL0
#define LOG_LOCAL0 (16<<3)
#define LOG_LOCAL1 (17<<3)
#define LOG_LOCAL2 (18<<3)
#define LOG_LOCAL3 (19<<3)
#define LOG_LOCAL4 (20<<3)
#define LOG_LOCAL5 (21<<3)
#define LOG_LOCAL6 (22<<3)
#define LOG_LOCAL7 (23<<3)
#define LOG_EMERG    0
#define LOG_ALERT    1
#define LOG_CRIT     2
#define LOG_ERR      3
#define LOG_WARNING  4
#define LOG_NOTICE   5
#define LOG_INFO     6
#define LOG_DEBUG    7
#endif

#ifndef WIN32
#include "syslog.h"
#endif

#include <string.h>

namespace woodycxx { namespace sysipc
{

    const uint num_types = 32;
    event_log* debug_log::debug_streams = 0;
    event_log* application_log::app_streams = 0;

    //-------------------------------------------------------------------------------------
    event_log::event_log( const char* nm ) : _filename( nm ), _level( (levels)0 )
    { 
        memset( masks, 0, sizeof( masks ) ); 
    }

    //-------------------------------------------------------------------------------------
    void event_log::write_mask( levels lev, uint m )
    {
        // each level should be an or of all previous levels
        for ( uint i = lev; i < num_levels; i++ )
            masks[ i ] |= m;

        _streams.mask = masks[ _level ];
    }

    //-------------------------------------------------------------------------------------
    void event_log::open_file()
    {
        if ( (!_file.is_open() ) && ( _filename.length() > 0  ) && ( !_streams.log_to_file ) )
        {
            _file.open( _filename.c_str(), ios_base::out | ios_base::app );
            if ( _file.is_open() )
            {
                _streams.file_strm = &_file;
                _streams.log_to_file = true;
            }
        }
    }

    //-------------------------------------------------------------------------------------
    void event_log::close_file()
    {
        if ( _file.is_open() )
        {
            _file.close();
            if ( _file.is_open() )
            {
                _streams.file_strm = 0;
                _streams.log_to_file = false;
            }
        }
    }

#ifdef WIN32
    void openlog(const char* ident, int option, int facility)
    {
    }
    void closelog(void)
    {
    }
    void syslog (int, const char *, ...)
    {
    }
#endif

    //-------------------------------------------------------------------------------------
    //  open_syslog("bmc_app", LOG_LOCAL1)
    //-------------------------------------------------------------------------------------
    void event_log::open_syslog(const char* ident, int log_localn)
    {
        openlog(ident, 0, log_localn);
        _streams.log_to_syslog = true;
        _streams.facility = log_localn;
    }

    //-------------------------------------------------------------------------------------
    void event_log::close_syslog()
    {
        closelog();
        _streams.log_to_syslog = false;
        _streams.facility = -1;
    }

    //-------------------------------------------------------------------------------------
    void event_log::streams::append( const char* p, const string& msg )
    {
        crit_sec.enter();

        sysipc::timestamp   stmp;

        if ( string::npos == msg.find_first_not_of( " " ) )
        {
            cout << "empty string...";
        }

        if ( log_to_file  )
        {
            *file_strm << stmp << " " << prefix( p ) << " " << msg << endl;
            file_strm->flush();
        }

        if ( log_to_stdout )
        {
            cout << stmp << " " << prefix( p ) << " " << msg << endl;
            cout.flush();
        }

        if ( log_to_syslog )
        {
            // todo: log level (warning, info, etc should be passed to this function, 
            syslog (LOG_WARNING, msg.c_str());
        }
        crit_sec.leave();
    }

} // end namespace
} // end woodycxx namespace