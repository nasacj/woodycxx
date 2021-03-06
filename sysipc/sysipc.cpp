/*
  Copyright (c) 2014-2015 by NASa Qian <nasacj@nasacj.net>
  This file is part of the woodycxx library.

  This software is distributed under BSD 3-Clause License.
  The full license agreement can be found in the LICENSE file.

  This software is distributed without any warranty.
*/

#ifdef WIN32
#include <windows.h>
#else
//#include <sys/statfs.h>
#endif

#include "sysipc.h"
#include <iomanip>
#include <string.h>

namespace woodycxx {
namespace sysipc {
#ifdef WIN32
//-------------------------------------------------------------------------------------
//--------------------  Windows -------------------------------------------------------
//-------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------
ostream& operator<<( ostream& strm, const timestamp& stmp )
{
    char timestr[ 64 ];
    tm  t;

    memset(timestr, '\0', sizeof(timestr));

    if ( ( 0 == localtime_s( &t, &stmp.sys_time ) ) && ( 0 != strftime( timestr, sizeof( timestr ) - 1, "%H:%M:%S", &t ) ) )
        strm << "[" << timestr << "." << setfill('0') << setw( 3 ) << stmp.msecs << "]";
    else
        strm << "invalid timestamp";
    return strm;
}


ostream& operator<<( ostream& strm, const os_error& err )
{
    strm << "system error, error= " << err._error_code;
    return strm;
}

//-------------------------------------------------------------------------------------
void timestamp::touch()
{
    sys_time = time( NULL );
    SYSTEMTIME  systime;
    GetSystemTime( &systime );
    msecs = systime.wMilliseconds;
}

#else
//-------------------------------------------------------------------------------------
//--------------------  Linux   -------------------------------------------------------
//-------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------
ostream &operator<<(ostream &strm, const timestamp &stmp) {
  char timestr[64];
  tm t;

  memset(timestr, '\0', sizeof(timestr));

  if ((NULL != localtime_r(&stmp.sys_time, &t)) && (0 != strftime(timestr, sizeof(timestr) - 1, "%H:%M:%S", &t)))
    strm << "[" << timestr << "." << setfill('0') << setw(3) << stmp.msecs << "]";
  else
    strm << "invalid timestamp";
  return strm;
}





//ostream& operator<<( ostream& strm, const os_error& err )
//{
//    strm << "system error, error= " << strerror( err._error_code );
//    return strm;
//}

//-------------------------------------------------------------------------------------
void timestamp::touch() {
  sys_time = time(NULL);

  timeval temp;
  gettimeofday(&temp, NULL);
  sys_time = temp.tv_sec;
  msecs = temp.tv_usec / 1000;
}

#endif
}
}
