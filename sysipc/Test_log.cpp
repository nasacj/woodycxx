#include "sysipc_event_log.h"

#ifndef WIN32
#define USE_SYSLOG
#include <syslog.h>
#endif

using namespace woodycxx::sysipc;

int main()
{
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
    DEBUG_INFO("TEST", "Hello World!");
    return 0;
}
