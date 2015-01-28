/*
  Copyright (c) 2014-2015 by NASa Qian <nasacj@nasacj.net>
  This file is part of the woodycxx library.

  This software is distributed under BSD 3-Clause License.
  The full license agreement can be found in the LICENSE file.

  This software is distributed without any warranty.
*/

#ifndef __SYSIPC_H
#define __SYSIPC_H

#include <string>
#include <vector>
#include <algorithm>
#include <iostream>

#ifdef WIN32
#define SKIP_WIN32(...)
#define SKIP_LINUX(...) __VA_ARGS__
#include <time.h>
#else
#define DLLIMPORT
#define DLLEXPORT
#include <sys/time.h>
#define SKIP_WIN32(...) __VA_ARGS__
#define SKIP_LINUX(...)
#endif

using namespace std;

namespace woodycxx { namespace sysipc
{

    /// \defgroup sysipc System Interprocess Communication
    /// \file sysipc.h
    /// \brief System IPC Header File

    typedef unsigned int    uint;
    typedef unsigned char   uint8;
    typedef unsigned char   byte;
    typedef unsigned short  uint16;
    typedef unsigned short  word;
    typedef unsigned long   uint32; 
    typedef unsigned long   dword; 

    struct os_error
    {
        uint    _error_code;

        os_error( uint i = 0 ) : _error_code( i ){}
        os_error& operator=( uint i ){ _error_code = i; return *this; }
        bool operator==( uint i ) const { return _error_code == i; }
    };

    ostream& operator<<( ostream& strm, const os_error& err );

    //-------------------------------------------------------------------------------------------
    class timestamp
    {
        friend ostream& operator<<( ostream&, const timestamp& stmp );
        friend bool operator<( const timestamp& l, const timestamp& r );

    public:
        time_t  sys_time;
        word    msecs;

        void clear(){ sys_time = 0; msecs = 0; }
        timestamp(){ touch(); }
        timestamp( time_t t, word ms ) : sys_time( t ), msecs( ms ){}
        void touch();
    };

    //-------------------------------------------------------------------------------------------
    inline bool operator<( const timestamp& l, const timestamp& r )
    {
        return ( l.sys_time < r.sys_time ) || ( ( l.sys_time == r.sys_time ) && ( l.msecs < r.msecs ) ); 
    }


    //-------------------------------------------------------------------------------------------
    inline uint diff(const timestamp& l, const timestamp& r )
    {
        uint d = (uint) ( l.sys_time - r.sys_time );
        if ( l.msecs < r.msecs )
            d--;
        return d;
    }

    //-------------------------------------------------------------------------------------------
    inline uint ms_diff(const timestamp& l, const timestamp& r )
    {
        uint d = (uint) ( l.sys_time - r.sys_time );
        if ( l.msecs < r.msecs )
            return ( d - 1 ) * 1000 + ( r.msecs - l.msecs );
        else
            return d * 1000 + ( l.msecs - r.msecs );
    }

    ostream& operator<<( ostream& strm, const timestamp& stmp );


    //-------------------------------------------------------------------------------------------

} // end namespace
} // end namespace woodycxx
#endif
