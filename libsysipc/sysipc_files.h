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
#ifndef __SYSIPC_FILES_H
#define __SYSIPC_FILES_H

#include "sysipc.h"
#include <fstream>
#include <sstream>
#include <list>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef WIN32
#include <windows.h>
#include <io.h>

// disable warning: 'this' : used in base member initializer list 
#pragma warning (disable:4355)

// disable warning: ...was declared deprecated
#pragma warning (disable:4996)

#else
#include <dirent.h>
#include <fnmatch.h>
#define file_read read
#define file_open open
#define file_close close
#define FHANDLE int
#define BAD_FHANDLE -1
#endif

/// \ingroup sysipc
/// \file sysipc_files.h
/// \brief File System Support

namespace sysipc
{

//------------------------------------------------------------------------------------------------------------------
class file
{
public:

    static bool calculate_md5_checksum( string* checksum, const char* fname, uint offset = 0, uint length = 0xFFFFFFFF );

    static uint size( const char* fn )
    { 
        struct stat stat_s;
        if ( 0 == stat( fn, &stat_s) )
            return stat_s.st_size;
        else
            return 0;
    }


    static string full_name( const string& path, const char* name )
    {
        string tmp( path );
        tmp += "/";
        tmp += name;
        return tmp;
    }

    static bool exists( const char* fn )
    {
        struct stat stat_s;
        return 0 == stat( fn, &stat_s);
    }

    static int copy(const string& src, const string& dest)
    {
        ifstream file_src(src.c_str(), fstream::binary);
        ofstream file_dest(dest.c_str(), fstream::trunc|fstream::binary);

        if ( !file_src.is_open() || !file_dest.is_open())
        {
            file_src.close();
            file_dest.close();
            ::remove(dest.c_str());
            return -1;
        }

        file_dest << file_src.rdbuf();

        if ( file_dest.bad() )
        {
            file_src.close();
            file_dest.close();
            ::remove(dest.c_str());
            return -1;
        }

        file_dest.flush();
        file_src.close();
        file_dest.close();
        return 0;
    }
};

class file_notifier
{
	string filename_being_watched;
	bool watch_enabled;
	bool first_wait;
	bool triggered;
    bool pending_alert;
	time_t last_timestamp;
public:
    file_notifier( const char* fn, uint32 mask );
	~file_notifier();
    void enable_watch();
    void disable_watch();
	void trigger();
    void wait();
    bool is_pending() {return pending_alert;};
    void clear_pending() {pending_alert = false;};
};

//------------------------------------------------------------------------------------------------------------------
/// \ingroup sysipc
/// \brief file
class text_file
{
protected:
    string              _name;
    ios_base::openmode  _mode;
    ofstream            _file;

    text_file( const string& name , ios_base::openmode mode ) :
        _name( name ), _mode( mode ), _file( _name.c_str(), _mode )
    { _file.close(); }

    text_file( const char* name , ios_base::openmode mode ) :
        _name( name ), _mode( mode ), _file( _name.c_str(), _mode )
    { _file.close(); }

    void open( ios_base::openmode mode )
    { 
       _file.open( _name.c_str(), _mode | mode );
    }
protected:
    void close(){ _file.close(); }
    bool is_open() { return _file.is_open(); }
};

//------------------------------------------------------------------------------------------------------------------
/// \ingroup sysipc
/// \brief tar_archive
class tar_archive
{
public:
    struct file_header
    {
        char    name[ 100 ];
        char    mode[ 8 ];
        char    uid[ 8 ];
        char    gid[ 8 ];
        char    size[ 12 ];
        char    mtime[ 12 ];
        char    chksum[ 8 ];
        char    typeflag[ 1 ];
        char    linkname[ 100 ];
        char    magic[ 6 ];
        char    version[ 2 ];
        char    uname[ 32 ];
        char    gname[ 32 ];
        char    devmajor[ 8  ];
        char    devminor[ 8  ];
        char    prefix[ 155 ];
    };
    static const uint      BLOCK_SIZE = 512;
    
    struct block
    {
    public:
        byte data[ BLOCK_SIZE ];
    };
    

    struct file
    {
        bool    valid;
        string  file_name;
        uint    file_size;
        uint    num_blocks;
        uint    block_number;

    public:
        file(): valid( false ), file_size( 0 ), num_blocks( 0 ), block_number( 0 ){}
        file( int fd  );
        bool is_valid() const { return valid; }
        bool is_eof() const { return num_blocks == block_number; }
        uint read( int fd, block* blk );
    };

private:
    int             fd;
    uint            archive_size;
    uint            position;
    file            current_file;
    
public:
    tar_archive() : archive_size( 0 ), position(){ fd = -1; }
    bool open( int d, uint size );
    void close();
    bool is_open() const {  return fd != -1; }
    bool next_file();
    uint read( byte* buffer, uint size );
    const file& current() const {  return current_file; }
    int get_fd() { return fd; }
    uint get_position() const { return position; }
    uint get_archive_size() const { return archive_size; }
    const string& get_current_file_name() const { return current_file.file_name; }
    static uint get_block_size() { return BLOCK_SIZE; }
};




//------------------------------------------------------------------------------------------------------------------
/// \ingroup sysipc
/// \brief directory 
class directory
{
    string          _name;
    os_error        _error;

public:
    static string  pwd();

    void set( const string& name );

    directory(){ _error = ~0; }
    directory( const char* name ){ set( string( name ) ); }
    bool exists() const
    { 
        struct stat stat_s;
        return 0 == stat( _name.c_str(), &stat_s);
    }
    string full_name( const char* nm ) const { string tmp( _name ); tmp += "/"; tmp += nm; return tmp; }
    const os_error& get_error(){ return _error; }

    void get_file_names(list<string>& filesindir);
    time_t last_timestamp(time_t passed_timestamp);

    template<class T>
    void for_each_file( T& functor, const char* pattern );
};
 

#ifdef WIN32
//-------------------------------------------------------------------------------------
template<class T>
void directory::for_each_file( T& functor, const char* pattern )
{
    if ( exists() )
    {
        WIN32_FIND_DATA entry;
        string search =  _name + "\\" + pattern;
         
        HANDLE h = FindFirstFile( search.c_str(), &entry );
        bool more = true;
        if ( h != INVALID_HANDLE_VALUE )
        {
            while ( more )
                more = functor( *this, entry.cFileName ) && FindNextFile( h, &entry );
            FindClose( h );
        }
    }
}


#else

//-------------------------------------------------------------------------------------
template<class T>
void directory::for_each_file( T& functor, const char* pattern )
{
    if ( exists() )
    {
        DIR* dir = opendir( _name.c_str() );
        if ( dir )
        {   
            struct dirent* entry = readdir( dir );
            while ( entry )
            {
                if ( 0 == fnmatch( pattern, entry->d_name, 0 ) )
                {
                    if ( functor( *this, entry->d_name ) )
                        entry = readdir( dir );
                    else
                        entry = 0;
                }
                else
                    entry = readdir( dir );
            }
            closedir( dir );
        }
    }
}
#endif


//------------------------------------------------------------------------------------------------------------------
/// \ingroup sysipc
/// \brief partition 
class partition
{
    friend ostream& operator<<( ostream& strm, const partition& part );
    string          _folder_name;

protected:
    string          _stat_file_name;
    uint            _space;
    os_error        _error;

    void get_stats();

public:
    /// \brief 
    partition( const char* fname ) : _folder_name( fname ), _stat_file_name( fname ), _space( 0 ){}
    virtual ~partition(){}

    bool is_mounted();
    uint            space() { get_stats(); return _space; }
    virtual void    mount(){}
};
ostream& operator<<( ostream& strm, const partition& part );


//------------------------------------------------------------------------------------------------------------------
/// \ingroup sysipc
/// \brief filesystem 
class file_system
{
public:
    static bool is_mounted_as( const char* device_name, const char* mount_name = 0);
};


#define WIN32_PATH_SLASH '\\'
#define LINUX_PATH_SLASH '/'
#define PATH_SLASH          LINUX_PATH_SLASH

//-------------------------------------------------------------------------------------------
/* convert slashes for pathnames to linux style */
inline const string convert_path_slashes(const string& _str)
{
    if (_str.find(WIN32_PATH_SLASH) != string::npos)
    {
        string str = _str;
        for ( unsigned int i=0; i < str.length(); i++ )
        {
            if ( str[i] == WIN32_PATH_SLASH ) str[i] = LINUX_PATH_SLASH;
        }
        return str;
    }
    return _str;
}

//-------------------------------------------------------------------------------------------
inline const string file_join( const string& p1, const string& p2)
{
    string str = p1;

    if ( p2[0] != '/' )
        str += PATH_SLASH ;

    str += p2;

    return str;
}

//-------------------------------------------------------------------------------------------
inline const string trim_end( const string& _str )
{
    string str;
    size_t p1 = _str.find_last_not_of( ' ' );
    str = _str.substr( 0, p1+1 );
    return str;
}


//-------------------------------------------------------------------------------------------
// 
struct std_directories {
    string name;
    string root_dir;

    string tmp_dir;
    string var_dir;
    string etc_dir;
    string bin_dir;
    string lib_dir;
    string pstorage;
    string test_dir;


    std_directories(const string& _name)
    {
        name = _name;
    }
    void set_root_dir(const string& test_root)
    {
        root_dir = convert_path_slashes(test_root);
        set_dir(tmp_dir, "tmp");
        set_dir(var_dir, "var");
        set_dir(etc_dir, "etc");
        set_dir(bin_dir, "bin");
        set_dir(lib_dir, "lib");
        set_dir(pstorage, "pstorage");
        set_dir(test_dir, "test");
    }
    string& set_dir(string& dir, const string& path)
    {
        dir = file_join(root_dir, path);
        return dir;
    }
};

} // end namespace

#endif
