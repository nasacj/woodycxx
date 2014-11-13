/******************************************************************************
 * 
 *  Licensed Materials - Property of IBM.
 * 
 *  (C) Copyright IBM Corporation 2009
 * 
 *  All Rights Reserved.
 * 
 *  US Government Users Restricted Rights -
 *  Use, duplication or disclosure restricted by
 *  GSA ADP Schedule Contract with IBM Corporation.
 * 
 *****************************************************************************/

#ifndef __SYSIPC_INOTIFY2_H
#define __SYSIPC_INOTIFY2_H

#include <map>
#include <list>
#include <fstream>
#include <sstream>

#ifndef WIN32
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <sys/inotify.h>
    #include <time.h>
    #include <unistd.h>
#endif

#include "sysipc.h"
#include "sysipc_event_log.h"
#include "sysipc_application_control.h"
#include "sysipc_exceptions.h"

#define MAX_WATCH_HANDLES 200
#ifdef WIN32
#define FLAG_DIR    "C:\\tmp\\sys_inot\\lock"
#else
#define FLAG_DIR    "/tmp/sys_inot/lock"
#endif
#define DELIM       "."

//------------------------------------------------------------------------------------------------------------------
namespace sysipc
{

    class inotifier
    {
    public:
        struct watch_elem;
        class callback
        {
        public:
            // see linux kernel include/inotify for details
            enum callback_oper
            {
                cbo_invalid  = 0x000,
                cbo_accessed = 0x001,
                cbo_modified = 0x002,
                cbo_attrib   = 0x004,
                cbo_close_wr = 0x008,
                cbo_close_ro = 0x010,
                cbo_open     = 0x020,
                cbo_move_frm = 0x040,
                cbo_move_to  = 0x080,
                cbo_created  = 0x100,
                cbo_deleted  = 0x200,
                cbo_del_slf  = 0x400,
                cbo_move_slf = 0x800
            };
            callback(){};
            virtual void on_inotify(watch_elem* elem, callback::callback_oper cbo) = 0;
        };

        class file_flag
        {
        private:
            string  wname;       // whole file name
            uint    cbo;         // the oper code want to care

            string  flag_wname;  // whole file name the flag
        public:
            file_flag( const string& whole_name, uint opcode = 0 );

            bool is_locked();
            void create_lock();
            void remove_lock();

        private:
            static string generate_flag_wname( const string& whole_name, uint opcode = 0 );
            static string get_wname( const string& flag_whole_name );
            static uint   get_cbo( const string& flag_whole_name );

            // remove dir/file to the specified top level,
            // stop if a upper level dir is not empty.
            static void remove_to_dir( const char* file, const char* top_dir );

            // create the whole dir tree. will create the parent if doesn't exist.
            static void create_dir_tree( const char* dir );

            // create the whole file path. will create the parent dir if doesn't exist.
            static void create_file( const char* file );

            // dir name of a file, no "/" at the end. e.g. /var/DS
            static string dirname( const char* file );

            // file name of a file, no "/" at the beginning. e.g. test.log
            static string basename( const char* file );
        };

        struct watch_elem
        {
            int os_file_wd;         // os file watch descriptor (for changes to existing files)
            int os_dir_wd;          // os  dir watch descriptor (for new files)
            string dname;           // dir path of watched file
            string fname;           // file name of watched file
            string wname;           // whole name of watched file
            bool file_exists;       // true if file exists
            bool file_modified;     // true if modified since last checked
            long long last_write;   // contains last time of file write time
            callback* cb;           // callback pointer
            int primary_mask;       // primary event mask (not changeable)
            int current_mask;       // current event mask (subset of primary)
            void clear_info()
            {
                file_exists = false;
                last_write = 0;
            }
        };

    private:
        sysipc::critical_section crit_sect;
        list<watch_elem> watches;

        void start_watcher_thread();
        sysipc::Tthread<inotifier>* watcher_thread;

        void watcher_proc();
        static void watcher_proc_static( inotifier* me )
        {
            me->watcher_proc();
        }
        virtual void reset(){}
        virtual void add_watch_os(watch_elem& elem){}
        virtual void wait_os(){}

    protected:
        bool watchlist_changed;
        void call_callback(watch_elem* pelem, callback::callback_oper cbo);
        watch_elem* find_watch_element(int os_id);
        watch_elem* find_watch_element(int os_id,  const string& fname);

    public:
        static inotifier* singleton;

        inotifier() : watcher_thread( 0 ), watchlist_changed( false ){}
        virtual ~inotifier(){}
        static inotifier* create();
        watch_elem* add_watch( const string& dname, const string& fname, int primary_mask, callback* cb);
        void mask(watch_elem* pelem, int secondary_mask);
        void unmask(watch_elem* pelem, int secondary_mask);

        static void global_mask( const string& wname, uint mask = 0 );
        static void global_unmask( const string& wname, uint mask = 0 );
        static bool is_global_maskoff( const string& wname, uint mask = 0 );
        
    };

    //------------------------------------------------------------------------------------------------------------------
#ifdef WIN32
    class inotifier_win32 : public inotifier
    {
        int ch_index;
        HANDLE  watch_handles[MAX_WATCH_HANDLES];

    public:
        inotifier_win32()
        {
            reset();
        }

        void reset()
        {
            ch_index = 0;
        }

        void add_watch_os(watch_elem& elem)
        {
            if ( ch_index >= MAX_WATCH_HANDLES )
            {
                cout << "inotifier, MAX_WATCH_HANDLES exceeded" << endl;
                SYS_ASSERT ( false );
            }
            else
            {
                elem.os_file_wd = ch_index;     // on windows, we see all changes with one watch/change handle
                elem.os_dir_wd  = ch_index;     // ... so just use the same one
                obtain_file_info( &elem, true );
                watch_handles[ch_index] = FindFirstChangeNotification( elem.dname.c_str(), false, FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE );
                // only accept valid watch descriptors
                if ( watch_handles[ch_index] != INVALID_HANDLE_VALUE )
                    ch_index++;
            }
        }

        void obtain_file_info(watch_elem* pelem, bool init_info)
        {
            HANDLE fh;

            if ( init_info )
                pelem->clear_info();

            // we are not modified until proven otherwise
            pelem->file_modified = false;

            fh = CreateFile(pelem->wname.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
            if ( fh != INVALID_HANDLE_VALUE )
            {
                BY_HANDLE_FILE_INFORMATION finfo;
                if ( GetFileInformationByHandle( fh, &finfo ) )
                {
                    // never set the modified flag on init
                    if ( !init_info )
                    {
                        // check if the file is newly created, or if not.. did the last write time increase
                        if ( !pelem->file_exists )
                            pelem->file_modified = true;
                        else
                            pelem->file_modified = ( CompareFileTime(&finfo.ftLastWriteTime, (FILETIME*) &(pelem->last_write) ) == 1 );
                    }
                    pelem->file_exists = true;
                    pelem->last_write = *((long long*)(&finfo.ftLastWriteTime));
                }
                CloseHandle( fh );
            }
            else
            {
                // ignore file deletions, other than to reset the file info
                switch ( GetLastError() )
                {
                    case ERROR_FILE_NOT_FOUND:
                    case ERROR_PATH_NOT_FOUND:
                        pelem->clear_info();
                        break;
                    case ERROR_SHARING_VIOLATION:
                        pelem->file_modified = true;
                        break;
                    default:
                        cout << "inotify, unknown error code:" << GetLastError() << endl;
                        break;
                }
            }
        }
        void wait_os()
        {
            int os_file_wd;

            // no valid watch descriptors, just return
            if ( ch_index == 0 )
                return;

            while ( !watchlist_changed )
            {
                os_file_wd = WaitForMultipleObjects(ch_index, watch_handles, false, 1000);
                if ( os_file_wd < ch_index )
                {
                    watch_elem* pelem;

                    //cout << "INOT: -> wd:" << os_file_wd << endl;

                    pelem = find_watch_element( os_file_wd );
                    if ( pelem )
                    {
                        bool did_exist = pelem->file_exists;
                        
                        obtain_file_info( pelem, false );
                        
                        // if this file is NOT globally masked off, check the mask in advance
                        if ( !is_global_maskoff( pelem->wname ) )
                        {
                            if ( !did_exist && pelem->file_exists )
                                if ( pelem->current_mask & callback::cbo_created )
                                    call_callback( pelem, callback::cbo_created );    

                            if ( did_exist && !pelem->file_exists )
                                if ( pelem->current_mask & callback::cbo_deleted )
                                    call_callback( pelem, callback::cbo_deleted );    

                            // do not use cbo_accessed... we don't have that capability on win32
                            if ( did_exist && pelem->file_exists )
                                if ( pelem->file_modified )
                                    if ( pelem->current_mask & callback::cbo_close_wr )
                                        call_callback( pelem, callback::cbo_close_wr );
                        }
                    }
                    FindNextChangeNotification( watch_handles[os_file_wd] );
                }
            }
        }

    };
#else /* WIN32 */
    class inotifier_linux : public inotifier
    {
        int ch_index;
        int inotify_fd;
        // Used to record whether a dir has already been watched,
        // map pair is <dir_name, watch_id>
        map<string, int> dir_handles;

    public:
        inotifier_linux()
        {
            reset();
        }

        void reset()
        {
            inotify_fd = inotify_init();
            ch_index = 0;
            dir_handles.clear();
        }

        void add_watch_os(watch_elem& elem)
        {
            int pri_mask;
            int watch_id = 0;

            if ( ch_index >= MAX_WATCH_HANDLES )
            {
                cout << "inotifier, MAX_WATCH_HANDLES exceeded" << endl;
                SYS_ASSERT ( false );
            }
            else
            {
                obtain_file_info( &elem, true );

                // linux cannot watch a file until it exists, so we watch the directory for
                // new files to be created under it. once the file exists, we will add the
                // file watcher as well. We will always watch the directory for deletes/creates too.

                map<string, int>::iterator it = dir_handles.find(elem.dname);
                if ( it != dir_handles.end() )
                {
                    watch_id = it->second;
                }
                else
                {
                    // we always need to know when a file is created or deleted... even though we may not notify the callback
                    pri_mask = ( callback::cbo_created | callback::cbo_deleted );

                    // add the directory watch handle if we are not already watching it
                    watch_id = inotify_add_watch( inotify_fd, elem.dname.c_str(), pri_mask );
                    if (watch_id > 0)
                        dir_handles[elem.dname] = watch_id;
                }

                if ( watch_id > 0 )
                {
                    elem.os_dir_wd = watch_id;

                    if ( elem.file_exists )
                    {
                        // add the file watch handle
                        elem.os_file_wd = inotify_add_watch( inotify_fd, elem.wname.c_str(), elem.primary_mask );
                        if ( elem.os_file_wd > 0 )
                            ch_index++;
                    }
                }
            }
        }
        
        void obtain_file_info(watch_elem* pelem, bool init_info)
        {
            int rc;
            struct stat statinfo;

            if ( init_info )
                pelem->clear_info();

            // we are not modified until proven otherwise
            pelem->file_modified = false;

            //fh = CreateFile(pelem->wname.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
            rc = stat( pelem->wname.c_str(), &statinfo );
            if ( rc == 0 )
            {
                // never set the modified flag on init
                if ( !init_info )
                {
                    // check if the file is newly created, or if not.. did the last write time increase
                    if ( !pelem->file_exists )
                        pelem->file_modified = true;
                    else
                        pelem->file_modified = ( difftime(statinfo.st_mtime, (time_t) pelem->last_write) > 0 );
                }
                pelem->file_exists = true;
                pelem->last_write = statinfo.st_mtime;
            }
            else
            {
                // ignore file deletions, other than to reset the file info
                pelem->clear_info();
            }
        }

        void wait_os()
        {
            int os_id;
            int len, ci;
            const int IN_EVENT_SIZE = sizeof (struct inotify_event);
            const int IN_BUF_LEN = (128 * (IN_EVENT_SIZE + 16));
            char in_buf[IN_BUF_LEN];
            inotify_event* in_evt;

            while ( !watchlist_changed )
            {
                //os_id = WaitForMultipleObjects(ch_index, watch_handles, false, 1000);
                len = read(inotify_fd, in_buf, IN_BUF_LEN);

                if ( len < 0 )
                {
                    // need to redo watchlist
                    if ( errno != EINTR )
                        break;
                }

                if ( len == 0 )
                    continue;


                ci = 0;
                while ( ci < len )
                {
                    in_evt = (struct inotify_event *) &in_buf[ci];

                    os_id = in_evt->wd;

                    {
                        watch_elem* pelem;
                        string fname("");

                        cout << "INOT: -> wd:" << in_evt->wd      << \
                                    " mask:" << hex << in_evt->mask << \
                                     " len:" << dec << in_evt->len  << \
                                    " name:" << (in_evt->len ? in_evt->name : "") << endl;
                        /*ostringstream strm;
                        strm << "INOT: -> wd:" << in_evt->wd      << \
                                    " mask:" << hex << in_evt->mask << \
                                     " len:" << dec << in_evt->len  << \
                                    " name:" << (in_evt->len ? in_evt->name : "");
                        sysipc::application_log::append( "inotify", strm.str() );*/
                                

                        // new files trigger directory changes, but contain the name of the new file
                        if ( in_evt->len > 0 )
                            fname = in_evt->name;
    
                        pelem = find_watch_element( os_id, fname );
                        if ( pelem )
                        {
                            cout << "FOUND ===> " << pelem->wname << endl;
							
                            callback::callback_oper cbo = (callback::callback_oper) in_evt->mask;
                            
                            obtain_file_info( pelem, false );

                            // whenever a file is created or deleted, we need to redo our inotify list
                            if ( cbo == callback::cbo_created || cbo == callback::cbo_deleted )
                                watchlist_changed = true;

                            // if this file is NOT globally masked off, check the mask in advance
                            if ( !is_global_maskoff( pelem->wname, cbo ) )
                            {
                            	cout << "!is_global_maskoff ===> " << pelem->wname << endl;
								cout << "pelem->current_mask = " << pelem->current_mask << " cbo = " << cbo << endl;
                                // if the caller wants notification, call the callback
                                if ( pelem->current_mask & cbo )
                                {	
                                	cout << "call_callback ===> " << pelem->wname << endl;
                                    call_callback( pelem, cbo );
                                }
                            }
                        }
                    }
                    
                    // process next event, if one exists
                    ci += IN_EVENT_SIZE + in_evt->len;
                }
            }

            // this will autoclose everything for us
            close( inotify_fd );
        }

    };
#endif /* WIN32 */
} /* namespace */

#endif /* __SYSIPC_INOTIFY2_H */

