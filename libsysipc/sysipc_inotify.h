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

#ifndef __SYSIPC_INOTIFY_H
#define __SYSIPC_INOTIFY_H

#ifndef WIN32

#include <map>
#include <list>
#include <fstream>
#include <sstream>

#include <fcntl.h>
#include <dirent.h>
#include <fnmatch.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include "sysipc.h"
#include "sysipc_event_log.h"
#include "sysipc_application_control.h"

namespace sysipc {

#define SELECT_MAX_FD(x,y) ((x) > (y) ? (x) : (y))

class linux_inotifier
{
private:
    /*
     *  Private class enums.
     */

    enum command_message {
        COMMAND_MESSAGE_TRIGGER
    };

    /*
     *  Data objects.
     */

    int ln_flags;                                       // Linux notify private flags.
    int inotify_fd;                                     // Interface to inotify.
    int commandr_fd;                                    // Interface to self (for trigger and other commands).
    int commandw_fd;                                    // Interface to self (for trigger and other commands).
    bool watch_enabled;                                 // Is the object being watched?
    bool pending_alert;                                 // Is an alert pending which has not yet been handled?

    pthread_mutex_t mutex;                              // Mutex.

    char *read_buffer;                                  // Memory object for kernel interface.
    string watched_object;                              // Name of the filesystem object.
    string watched_substring;
    map<string, int> watches;                           // Active watched objects;
    map<string, bool> working_table;                    // Work table to track modify bits before file close.
    list<string> dirty_object_queue;                    // Internal list to track modified objects.

    void init();

    enum command_message read_command_message (void);   // Internal command message processing.  

    void add_with_recursion (const char *name);         // Internal function to recurse.

    /*
     *  Prototypes.
     */

    void dispatch_inote_event (struct inotify_event *event);
    void wait_internal (string *out_string, unsigned int timeout_secs); 

    ofstream dbgfile;

#define INOTIFY_DEBUG_INIT do { string filename("/tmp/inotify_"); filename += watched_substring + ".log"; dbgfile.open(filename.c_str(), ios::out); } while (0);
#define INOTIFY_DEBUG(x) do { if (dbgfile.is_open()) dbgfile << x << endl; } while (0);

public:
    /*
     *  Public class enums.
     */

    /*
     *  Class macros
     */

    #define LN_FLAG_CARE_MODIFY 0x1

    /*
     *  Prototypes.
     */

    linux_inotifier();
    linux_inotifier (const char* fn, unsigned int bitmask);
    ~linux_inotifier ();

    bool set_params(const char* fn, unsigned int bitmask);  // set the params on a previous default contructed instantiation

    void wait ();                                       // Just wait for a file change.
    void wait (unsigned int timeout_secs);              // Just wait (With a timeout).
    void wait (string *out);                            // Wait for a file change and tell me which one it was.
    void wait (string *out, unsigned int timeout_secs); // Tell me which one it was with a timeout.

    void trigger ();                                    // Wake the select() up by force.
    void enable_watch ();
    void disable_watch ();

    bool is_pending() {return pending_alert;};          // Is an alert pending?
    void clear_pending() {pending_alert = false;};      // Alert has been handled, clear the alert
};






} /* namespace sysipc */

#endif /* _!WIN32 */
#endif /* __SYSIPC_INOTIFY_H */

