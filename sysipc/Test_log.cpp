/*
  Copyright (c) 2014-2015 by NASa Qian <nasacj@nasacj.net>
  This file is part of the woodycxx library.

  This software is distributed under BSD 3-Clause License.
  The full license agreement can be found in the LICENSE file.

  This software is distributed without any warranty.
*/

#include "sysipc_event_log.h"

#ifndef WIN32
#define USE_SYSLOG
#include <syslog.h>
#endif

using namespace woodycxx::sysipc;

int main()
{
/*
    debug_log dbg_lg("debug.log");
#ifdef USE_SYSLOG
    dbg_lg.open_syslog("Test_log", LOG_INFO);
#else
    dbg_lg.open_file();
#endif
    dbg_lg.set_mask(event_log::level_0, DEBUG_ERRORS);
    dbg_lg.set_mask(event_log::level_1, DEBUG_INFORMATION);
    dbg_lg.set_level(event_log::level_1);
    dbg_lg.enable_stdout();
    */
    DEBUG_LOGER.setFileName("debug.log");
    DEBUG_LOGER.open_syslog("Test_log", LOG_INFO);
    DEBUG_LOGER.set_mask(event_log::level_0, DEBUG_ERRORS);
    DEBUG_LOGER.set_mask(event_log::level_1, DEBUG_INFORMATION);
    DEBUG_LOGER.set_level(event_log::level_1);
    DEBUG_LOGER.enable_stdout();
    DEBUG_INFO("TEST", "Hello World!");
    return 0;
    
}

