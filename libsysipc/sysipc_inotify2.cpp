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

#include "sysipc_inotify2.h"
#include "sysipc_files.h"
#ifdef WIN32
#include <direct.h>
#endif

namespace sysipc
{

// singleton
inotifier* inotifier::singleton = 0;

#ifdef WIN32
    inotifier* inotifier::create() { return ( singleton ? singleton : singleton = new inotifier_win32() ); }
#else
    inotifier* inotifier::create() { return ( singleton ? singleton : singleton = new inotifier_linux() ); }
#endif

//------------------------------------------------------------------------------------------------------------------
inotifier::watch_elem* inotifier::add_watch( const string& _dname, const string& _fname, int primary_mask, callback* cb)
{
    watch_elem elem;

    string dname = convert_path_slashes( _dname );
    string fname = convert_path_slashes( _fname );

    elem.os_file_wd = -1;
    elem.dname = dname;
    elem.fname = fname;
    elem.wname = dname + PATH_SLASH + fname;
    elem.cb = cb;
    elem.primary_mask = primary_mask;
    elem.current_mask = primary_mask;
    
    watch_elem *p = NULL;
    crit_sect.enter();
    {
        watches.push_back( elem );
        p = &(watches.back());
        watchlist_changed = true;
        start_watcher_thread();
    }
    crit_sect.leave();

    return p;
}

//------------------------------------------------------------------------------------------------------------------
void inotifier::mask(watch_elem* pelem, int sec_mask)
{
    if ( pelem->primary_mask | sec_mask )
        pelem->current_mask &= ~sec_mask;
}

//------------------------------------------------------------------------------------------------------------------
void inotifier::unmask(watch_elem* pelem, int sec_mask)
{
    if ( pelem->primary_mask | sec_mask )
        pelem->current_mask |= sec_mask;
}

//------------------------------------------------------------------------------------------------------------------
void inotifier::watcher_proc()
{
    while ( true )
    {
        sysipc::thread::sleep(1000);

        reset();

        crit_sect.enter();
        {
            int size = watches.size();
            // kill thread if nothing to do
            if ( size == 0 )
                break;

            list<watch_elem>::iterator it;
            for ( it = watches.begin(); it != watches.end(); it++ )
                add_watch_os(*it);

            watchlist_changed = false;
        }
        crit_sect.leave();

        wait_os();
    }
    watcher_thread = 0;
}

//------------------------------------------------------------------------------------------------------------------
void inotifier::start_watcher_thread()
{
    if ( !watcher_thread )
    {
        int stack_size = 1024;
        watcher_thread = sysipc::Tthread<inotifier>::create( "inotifier_thread", &watcher_proc_static, this, stack_size);
        if ( watcher_thread )
            watcher_thread->run();
    }
}

//------------------------------------------------------------------------------------------------------------------
inotifier::watch_elem* inotifier::find_watch_element(int os_id)
{
    return find_watch_element( os_id, "" );
}

//------------------------------------------------------------------------------------------------------------------
inotifier::watch_elem* inotifier::find_watch_element(int os_id, const string& fname )
{
    list<watch_elem>::iterator it;
    watch_elem* rtn = 0;
    
    crit_sect.enter();
    for ( it=watches.begin(); it != watches.end(); it++ )
    {
        // if fname is not empty, it must match elem.fname (linux only)
        if ( fname != "" )
        {
            if ( ((*it).os_dir_wd == os_id) && ((*it).fname == fname) )
                rtn = &(*it);
        }
        else
        {   
            // on win32, we only need to check the file wd
            // on linux, only changed files will trigger the file wd
            if ( (*it).os_file_wd == os_id )
            {
                rtn = &(*it);
            }
        }
    }
    crit_sect.leave();

    return rtn;
}

//------------------------------------------------------------------------------------------------------------------
void inotifier::call_callback(watch_elem* pelem, callback::callback_oper cbo)
{
    if ( pelem )
        (*pelem).cb->on_inotify( pelem, cbo );
}

//------------------------------------------------------------------------------------------------------------------
void inotifier::global_mask( const string& wname, uint mask /* = 0 */ )
{
    file_flag ff( wname, 0 );   // temporarily disable the callback opcode support here due to the schedule. hardcoded to 0.
    ff.create_lock();
}

//------------------------------------------------------------------------------------------------------------------
void inotifier::global_unmask( const string& wname, uint mask /* = 0 */ )
{
    file_flag ff( wname, 0 );   // temporarily disable the callback opcode support here due to the schedule. hardcoded to 0.
    ff.remove_lock();
}

//------------------------------------------------------------------------------------------------------------------
bool inotifier::is_global_maskoff( const string& wname, uint mask /* = 0 */ )
{
    file_flag ff( wname, 0 );   // temporarily disable the callback opcode support here due to the schedule. hardcoded to 0.
    return ff.is_locked();
}

//------------------------------------------------------------------------------------------------------------------
/// class inotifier::file_flag
inotifier::file_flag::file_flag( const string& whole_name, uint opcode /* = 0 */ ) :
    wname( whole_name ),
    cbo( opcode )
{
    flag_wname = generate_flag_wname( wname, cbo );
}

//------------------------------------------------------------------------------------------------------------------
bool inotifier::file_flag::is_locked()
{
    ifstream ifs( flag_wname.c_str() );

    if( ifs )
    {
        ifs.close();
        return true;
    }
    else
        return false;
}

//------------------------------------------------------------------------------------------------------------------
void inotifier::file_flag::create_lock()
{
    // if flag exists, just exit
    if ( is_locked() )
    {
        return;
    }

    create_file( flag_wname.c_str() );
}

//------------------------------------------------------------------------------------------------------------------
void inotifier::file_flag::remove_lock()
{
    if( is_locked() )
    {
        remove_to_dir( flag_wname.c_str(), FLAG_DIR );
    }
}

//------------------------------------------------------------------------------------------------------------------
string inotifier::file_flag::generate_flag_wname( const string& whole_name, uint opcode /* = 0 */ )
{
    stringstream ss;

    ss << FLAG_DIR << whole_name << DELIM << opcode;

    return ss.str();
}

//------------------------------------------------------------------------------------------------------------------
string inotifier::file_flag::get_wname( const string& flag_whole_name )
{
    size_t pos = flag_whole_name.find_last_of( DELIM );
    return flag_whole_name.substr( 0, pos );
}

//------------------------------------------------------------------------------------------------------------------
uint inotifier::file_flag::get_cbo( const string& flag_whole_name )
{
    uint rc = 0;
    size_t pos = flag_whole_name.find_last_of( DELIM );

    if( pos == string::npos && pos == flag_whole_name.length() )
        rc = 0;
    else
    {
        stringstream ss;
        ss << flag_whole_name.substr( pos + 1 > flag_whole_name.length() ? string::npos : pos + 1 );
        ss >> rc;
    }
    return rc;
}

//------------------------------------------------------------------------------------------------------------------
void inotifier::file_flag::remove_to_dir( const char* file, const char* top_dir )
{
    string fwname = file;
    while( fwname != top_dir )
    {
        //cout << "removing " << fwname << " ... " << endl;
        int rc = remove( fwname.c_str() );
        if( rc )
        {
            //cout << "!error = " << rc << ", no = " << errno << endl;
            return;
        }
        fwname = fwname.substr( 0, fwname.find_last_of( "/\\" ) );
    }
    //cout << "done." << endl;
}

//------------------------------------------------------------------------------------------------------------------
void inotifier::file_flag::create_dir_tree( const char* dir )
{
    string dpath, path = dir;
    size_t pos_b = 0, pos_e;

    while( dpath != dir )
    {
        pos_e = path.find_first_of( "/\\", pos_b + 1 );
        dpath = path.substr( 0, pos_e );
        //cout << "creating dir " << dpath << " ... " << endl;

#ifndef WIN32
		int rc = mkdir( dpath.c_str(), S_IRWXU | S_IRWXG | S_IRWXO ); //S_ variables are linux only
#else
		int rc = _mkdir( dpath.c_str());
#endif

        if( rc == -1 && errno != EEXIST )
        {
            //cout << " !error = " << rc << ", no = " << errno << endl;
            return;
        }
        pos_b = pos_e;
    }
    //cout << "done." << endl;
}

//------------------------------------------------------------------------------------------------------------------
void inotifier::file_flag::create_file( const char* file )
{
    create_dir_tree( dirname( file ).c_str() );

    fstream fs;
    fs.open( file, ios_base::app );
    fs.close();
}

//------------------------------------------------------------------------------------------------------------------
string inotifier::file_flag::dirname( const char* file )
{
    string str = file;
    size_t pos = str.find_last_of( "/\\" );
    return str.substr( 0, pos );
}

//------------------------------------------------------------------------------------------------------------------
string inotifier::file_flag::basename( const char* file )
{
    string str = file;
    size_t pos = str.find_last_of( "/\\" );
    return str.substr( pos + 1 > str.length() ? string::npos : pos + 1 );
}


} /* namespace */
