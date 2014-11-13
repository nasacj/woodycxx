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

#ifndef WIN32

#include <unistd.h>
#include <inttypes.h>
#include <sys/statfs.h>
#include <sys/socket.h>
#include <sys/inotify.h>

#include <map>
#include "sysipc_files.h"
#include "sysipc_threads.h"
#include "sysipc_inotify.h"
#include "sysipc_event_log.h"

namespace sysipc {

/*
 *  Private interface code.
 */

void
linux_inotifier::dispatch_inote_event(struct inotify_event *event)
{
    /*
     *  Filter out events that happened to directory objects.
     */

    if (event->mask & IN_ISDIR)
        return;

    /*
     *  If the event is a modify start to track it.  Must be before CLOSE_WRITE
     *  check.
     */

    pthread_mutex_lock(&mutex);

    if (event->mask & IN_MODIFY)
    {
        working_table[event->name] = true;
    }

    /*
     *  If the event is a close write send an alert if the data underneath it 
     *  it was modified.
     */

    if (event->mask & IN_CLOSE_WRITE)
    {
        if ((this->ln_flags & LN_FLAG_CARE_MODIFY) == 0)
        {
            dirty_object_queue.push_back(event->name);
        } else {
            if (working_table.find(event->name) != working_table.end())
            {
                /*
                 *  Push the dirty object to the back of the dirty_object_queue and 
                 *  remove it from the tracking list.
                 */

                dirty_object_queue.push_back(event->name);
                working_table.erase(event->name);
            }
        }
    }

    pthread_mutex_unlock(&mutex);

}

enum linux_inotifier::command_message
linux_inotifier::read_command_message(void)
{
    int rv;
    enum linux_inotifier::command_message command;

    rv = read(commandr_fd, &command, sizeof(command));

    return command;
}

void
linux_inotifier::wait_internal(string *out_string, uint32_t timeout)
{
//    INOTIFY_DEBUG("linux_inotifier::wait_internal(string, " << timeout << ")");
    fd_set read_set;
    list<string>::iterator current;
    int rc, max_fd, my_bytes, read_bytes;

    /*
     *  Get the MAX fd number for select.
     */

    max_fd = 0;
    max_fd = SELECT_MAX_FD(inotify_fd, max_fd);
    max_fd = SELECT_MAX_FD(commandr_fd, max_fd);

    while (1)
    {
        uint32_t event_buffer_pos;

        /*
         *  At the top of the loop, check to see if any dirty objects have
         *  shown up in the working queue.  If so they must be poped off.
         */

        pthread_mutex_lock(&mutex);

        current = dirty_object_queue.begin();

        if (current != dirty_object_queue.end())
        {
            /*
             *  Assign the out object if we care about it.
             */

            if (out_string)
                out_string->assign(dirty_object_queue.front());

            /*
             *  Pop the front off the queue and return.
             */

            dirty_object_queue.pop_front();
            pending_alert = true;

//            while (!dirty_object_queue.empty()) dirty_object_queue.pop_front();

            pthread_mutex_unlock(&mutex);
            return;
        }

        pthread_mutex_unlock(&mutex);

        /*
         *  Block on the inotify socket until we have data. 
         */

        FD_ZERO(&read_set);
        FD_SET(inotify_fd, &read_set);
        FD_SET(commandr_fd, &read_set);

        if (timeout)
        {
            struct timeval tv;
            tv.tv_sec = timeout;
            tv.tv_usec = 0;

            rc = select(max_fd + 1, &read_set, NULL, NULL, &tv);

        } else {
            rc = select(max_fd + 1, &read_set, NULL, NULL, NULL);
        }

        /*
         *  If rc is 0 a select() timeout happened.
         */

        if (rc == 0)
        {
            pending_alert = true;
//            INOTIFY_DEBUG("Timeout occurred");
            return;
        }

        if (rc < 0)
            continue;

        /*
         *  Was it an event on the command fd?
         */

        if (FD_ISSET(commandr_fd, &read_set))
        {
            enum linux_inotifier::command_message command;
            command = this->read_command_message();

            if (command == COMMAND_MESSAGE_TRIGGER)
            {
                pending_alert = true;
                return; 
            }

            continue;
        }

        /*
         *  Make sure that the inotify_fd was set, if not continue.  
         *  (Saves space for indentation).
         */

        if (!FD_ISSET(inotify_fd, &read_set))
            continue;

        /*
         *  Make sure there is enough data to read.
         */

        rc = ioctl(inotify_fd, FIONREAD, &read_bytes);

        if (rc < 0)
            continue;

        if (read_bytes < (int)sizeof(struct inotify_event))
            continue;

        if (!FD_ISSET(inotify_fd, &read_set))
            continue;

        /*
         *  Read the event.  It may be required to implement this in
         *  a while loop, updating a bytes read counter, if truncated
         *  events happen.
         */

        memset(read_buffer, 0, read_bytes);

        my_bytes = read(inotify_fd, read_buffer, read_bytes);

        if (my_bytes != read_bytes)
        {
//            INOTIFY_DEBUG("Read error " << my_bytes << " != " << read_bytes); 
            continue;
        }

        /*
         *  Dispatch each event we read from the event queue.
         */

        event_buffer_pos = 0;

        while (my_bytes > 0)
        {
            struct inotify_event *event;

            event = (struct inotify_event *)&read_buffer[event_buffer_pos];

            /*
             *  Dispatch this specific event.
             */

            this->dispatch_inote_event(event);

            if (event->len < 0 || event->len > MAXNAMLEN)
            {
                DEBUG_INFO("FS", "INOTE: Malformed inotify event:" << event->len << "\n");
                continue;
            }

            event_buffer_pos += sizeof(struct inotify_event) + event->len;
            my_bytes         -= sizeof(struct inotify_event) + event->len;
        }
    }

    // should not get here, but just in case
    pending_alert = true;
}

/*
 *  Public interface code.
 */

linux_inotifier::linux_inotifier() {
    inotify_fd  = 0;
    commandr_fd = 0;
    commandw_fd = 0;

    watch_enabled  = false;
    pending_alert = false;
    watched_object = "";

    pthread_mutex_init(&mutex, NULL);

    INOTIFY_DEBUG_INIT;

    INOTIFY_DEBUG("linux_notifer::linux_inotifer()");

    // don't call init yet, we need a set_params call later
}

linux_inotifier::linux_inotifier(const char *fn, unsigned int mask)
{
    ln_flags    = mask;
    inotify_fd  = 0;
    commandr_fd = 0;
    commandw_fd = 0;

    cout << "Thread: " << getpid() << " Watch object created: " << fn << endl;

    watch_enabled  = false;
    pending_alert = false;
    watched_object = fn;

    pthread_mutex_init(&mutex, NULL);

    INOTIFY_DEBUG_INIT;

    INOTIFY_DEBUG("linux_notifer::linux_inotifer(" << fn << ")");

    init();
}

bool linux_inotifier::set_params(const char *fn, unsigned int mask)
{
    pthread_mutex_lock(&mutex);

    if (watched_object.compare("") != 0) 
    {
        pthread_mutex_unlock(&mutex);
        return false;
    }

    ln_flags       = mask;
    watched_object = fn;

    INOTIFY_DEBUG("linux_notifer::set_params(" << fn << ")");

    pthread_mutex_unlock(&mutex);

    init();
    return true;
}

void linux_inotifier::init() {
    if (watched_object.find_last_of("/") != string::npos) 
    {
        watched_substring = watched_object.substr(watched_object.find_last_of("/")+1);
    } else {
        watched_substring = watched_object;
    }

    /*
     *  Init inotify.
     */

    inotify_fd = inotify_init();

    if (!inotify_fd)
        return;

    /*
     *  Create the socket pair to wake the object out of select() if need be.
     */
    int sockets[2];

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) != 0)
    {
        close(inotify_fd);
        return;
    }

    commandr_fd = sockets[0];
    commandw_fd = sockets[1];

    /*
     *  Allocate a storage area for read().  A page should be enough space.
     */

    read_buffer = (char *)calloc(1, 4096);

    /*
     *  Finally enable the watch.
     */

    this->enable_watch();
}

linux_inotifier::~linux_inotifier() 
{
    if (inotify_fd)
        close(inotify_fd);

    if (commandr_fd)
        close(commandr_fd);

    if (commandw_fd)
        close(commandw_fd);
}

void
linux_inotifier::add_with_recursion(const char *name)
{
    int error;
    struct stat sb;
    string nextdirstr;
    list<string> filesindir;
    list<string>::iterator it;

    cout << "Thread: " << getpid() << " Adding with recursion: " << name << endl;

    directory thisdir = directory(name);
    thisdir.get_file_names(filesindir);

    pthread_mutex_lock(&mutex);
    watches[name] = inotify_add_watch(inotify_fd, name, IN_ISDIR | IN_CLOSE_WRITE | IN_MODIFY);
    pthread_mutex_unlock(&mutex);

    for (it = filesindir.begin(); it != filesindir.end(); it++)
    {
        if ((it->compare(".") == 0) ||
            (it->compare("..")) == 0)
            continue;

        nextdirstr = thisdir.full_name((*it).c_str());

        error = stat(nextdirstr.c_str(), &sb);

        if (error)
            continue;

        if (S_ISDIR(sb.st_mode))
        {
            add_with_recursion(nextdirstr.c_str());
        }
    }
}

void 
linux_inotifier::enable_watch() 
{
    int error;
    struct stat sb;

    cout << "Thread: " << getpid() << " Enable watch: " << watched_object.c_str() << endl;

    /*
     *  If the watch is currently enabled the ::enable method should do nothing.
     */

    if (watch_enabled)
        return;

    error = stat(watched_object.c_str(), &sb);

    if (error)
        return;

    /*
     *  If the object is a directory traverse it and all watches for each
     *  child directory.
     *
     *  XXX:  TODO:  It would be nice to have control over the traverse depth.
     */

    if (S_ISDIR(sb.st_mode)) 
    {
        list<string> filesindir;
        list<string>::iterator it;

        directory thisdir = directory(watched_object.c_str());

        thisdir.get_file_names(filesindir);

cout << "Finished get_file_names" << endl;

        for (it = filesindir.begin(); it != filesindir.end(); it++)
        {
            if ((it->compare(".") == 0) ||
                (it->compare("..")) == 0)
                continue;

            string nextdirstr = thisdir.full_name((*it).c_str());

            /*
             *  If the target is a directory add it with recursion.
             */

            error = stat(nextdirstr.c_str(), &sb);

            if (error)
                continue;

            if (S_ISDIR(sb.st_mode)) 
            {
                add_with_recursion(nextdirstr.c_str());
                cout << "Thread: " << getpid() << " Calling add with recursion on: " << nextdirstr.c_str() << endl;
            }
        }
    }

    /*
     *  XXX:  TODO:  It would be nice to have further granularity to the inotify flags.
     */

    pthread_mutex_lock(&mutex);
    watches[watched_object.c_str()] = inotify_add_watch(inotify_fd, watched_object.c_str(), IN_ISDIR | IN_CLOSE_WRITE | IN_MODIFY);
    pthread_mutex_unlock(&mutex);

    watch_enabled = true;
    pending_alert = false;
}

void 
linux_inotifier::disable_watch() 
{
//    INOTIFY_DEBUG("linux_inotifier::disable_watch()");

    list<string>::iterator lcur;
    map<string, int>::iterator mcur;
    map<string, bool>::iterator wcur;

    cout << "Thread: " << getpid() << " Watch object destroyed: " << watched_object.c_str() << endl;

    /*
     *  Disable the watch and remove the watch device handle.
     */

    watch_enabled = false;

    pthread_mutex_lock(&mutex);

    for (mcur = watches.begin(); mcur != watches.end(); mcur++)
    {
        //cout << "Remove watch for: " << mcur->first << "\n";
        (void)inotify_rm_watch(inotify_fd, mcur->second);
    }

    watches.erase(watches.begin(), watches.end());

    /*
     *  Flush the dirty object queue.
     */

    while (!dirty_object_queue.empty()) 
        dirty_object_queue.pop_front();

    //  Flush the working table.

    working_table.erase(working_table.begin(), working_table.end());

    pthread_mutex_unlock(&mutex);
}

void
linux_inotifier::trigger()
{
//    INOTIFY_DEBUG("linux_inotifier::trigger()");
    enum command_message command = COMMAND_MESSAGE_TRIGGER;

    if (!commandw_fd)
        return;

    DEBUG_INFO("FS", "INOTE: trigger called.");

    //  Write the command message to the write FD.

    write(commandw_fd, &command, sizeof(command));
}

/*
 *  Overloaded interfaces to wait_internal.
 */

void
linux_inotifier::wait()
{
//    INOTIFY_DEBUG("linux_inotifier::wait()");
    wait_internal(NULL, 0);
}

void
linux_inotifier::wait(unsigned int timeout_secs)
{
//    INOTIFY_DEBUG("linux_inotifier::wait(" << timeout_secs << ")");
    wait_internal(NULL, timeout_secs);
}

void
linux_inotifier::wait(string *out)
{
//    INOTIFY_DEBUG("linux_inotifier::wait(string)");
    wait_internal(out, 0);   
    sleep(4);
}

void
linux_inotifier::wait(string *out, unsigned int timeout_secs)
{
//    INOTIFY_DEBUG("linux_inotifier::wait(string, " << timeout_secs << ")");
    wait_internal(out, timeout_secs); 
}


} /* namespace sysicp */

#endif /* !WIN32 */
