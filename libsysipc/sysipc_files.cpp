/******************************************************************************
 * 
 *  Licensed Materials - Property of IBM.
 * 
 *  (C) Copyright IBM Corporation 2008,2009
 * 
 *  All Rights Reserved.
 * 
 *  US Government Users Restricted Rights -
 *  Use, duplication or disclosure restricted by
 *  GSA ADP Schedule Contract with IBM Corporation.
 * 
 *****************************************************************************/
#ifdef WIN32
    #include <windows.h>
    #include <cstdio>
    #include <io.h>
#else
    #include <sys/inotify.h>
    #include <sys/statfs.h>
    #include <errno.h>
    #include <unistd.h>
#endif

#include <string.h>
#include <list>
#include <sys/stat.h>
#include "sysipc_threads.h"


#ifdef WIN32
    #define MTAB_FILE "c:\\ibmc\\sysserv\\sysipc\\etc\\mtab"
    #define __S_ISTYPE(mode, mask)  (((mode) & _S_IFMT) == (mask))
    #define S_ISDIR(mode)    __S_ISTYPE((mode), _S_IFDIR)

#else

    #if __X86 == 1
        #define MTAB_FILE "/home/nalds/cvs/ibmc/sysserv/sysipc/etc/mtab" 
    #else
        #define MTAB_FILE "/proc/mounts"
    #endif
#endif

#include "sysipc_files.h"
#include "sysipc_event_log.h"

namespace sysipc
{

    //-------------------------------------------------------------------------------------
    bool tar_archive::open( int d, uint file_size )
    {
        bool rtn = false;

        if ( ( !is_open() ) && ( file_size >= BLOCK_SIZE ) )
        {
            file f( d );
            if ( f.is_valid() )
            {
                current_file = f;
                archive_size = file_size;
                fd = d;
                rtn = true;
            }
        }
        return rtn;
    }

    //-------------------------------------------------------------------------------------
    void tar_archive::close()
    {
        fd = -1;
        archive_size = 0;
        current_file = file();
    }

    //-------------------------------------------------------------------------------------
    bool tar_archive::next_file()
    {
        bool rtn = false;

        if ( is_open() )
        {
            block   buffer;
            while ( !current_file.is_eof() )
                position += current_file.read( fd, &buffer );

            current_file = file( fd );
            if ( current_file.is_valid() )
                rtn = true;
        }
        //if ( rtn )
        //    cout << "next file" << endl;
        //else
        //    cout << "next file, end of file" << endl;
        //cout.flush();
        return rtn;
    }


    //-------------------------------------------------------------------------------------
    uint tar_archive::read( byte* buffer, uint size )
    {
        uint bytes_read = 0; 

        if ( !current_file.is_eof() )
        {
            uint nblks = size / BLOCK_SIZE ;

            // need to read integral number of blocks
            if ( ( size % BLOCK_SIZE ) > 0 )
                nblks++;

            block*  blk = ( block* ) buffer;

            while ( nblks-- > 0 )
            {
                uint n = current_file.read( fd, blk++ );

                // eof means end of the file within the tar file, not eof overall.
                // eof on the file means we read the number of 512 blocks in this file.
                if ( current_file.is_eof() )
                {
                    // because we are reading an integral number of blocks, adjust for padding
                    bytes_read += current_file.file_size % BLOCK_SIZE;
                    if ( ( current_file.file_size % BLOCK_SIZE ) == 0 )
                        bytes_read += BLOCK_SIZE;

                    break;
                }
                bytes_read += n;
            }
        }
        return bytes_read;
    }

    //-------------------------------------------------------------------------------------
    tar_archive::file::file( int fd ) :
    valid( false ), file_size( 0 ), num_blocks( 0 ), block_number( 0 )
    {
        block   header_block;
        uint n = ::read( fd, header_block.data, BLOCK_SIZE );
        if ( n == BLOCK_SIZE )
        {
            file_header* hdr = (file_header*) header_block.data;
            // may want to check header checksum at some point

            // check for ustar magic
            if ( 0 == strncmp( hdr->magic, "ustar", 5 ) )
            {
                char*   end_ptr;
                char    temp[ 13 ];
                memcpy( temp, hdr->size, 12 );
                temp[ 12 ] = 0;
                file_size = strtoul( temp, &end_ptr, 8 );
                file_name.assign( hdr->name );
                num_blocks = file_size / BLOCK_SIZE;
                if ( ( file_size % BLOCK_SIZE ) > 0 )
                    num_blocks++;
                valid = true;
            }
        }
    }

    //-------------------------------------------------------------------------------------
    uint tar_archive::file::read( int fd, block* blk )
    {
        uint bytes_read = 0;

        // this function reads up to a block (512 bytes) of data per call. The only case where it reads less 
        // is if its endof file which is really an error as tar files are padded to integral number of blocks
        if ( !is_eof() )
        {
            while ( bytes_read < BLOCK_SIZE )
            {
                uint n = ::read( fd, blk->data + bytes_read, BLOCK_SIZE - bytes_read );

                if ( n == 0 )
                    break;

                bytes_read += n;

            }
            block_number++;
        }
        return bytes_read;
    }


    //-------------------------------------------------------------------------------------
    ostream& operator<<( ostream& strm, const partition& part )
    {
        strm << "Blocks avaialble: " << part._space << endl;
        return strm;
    }


    //-------------------------------------------------------------------------------------
    bool file_system::is_mounted_as( const char* device_name, const char* mount_name )
    {
        bool rtn = false;

        // on Linux look in the /etc/mtab
        ifstream    mtab( MTAB_FILE, ios_base::in );
        string      device;
        string      mount;
        while ( ( !mtab.fail() ) && ( !mtab.eof() ) )
        {
            mtab >> device;
            if ( device.compare(device_name) == 0 )
            {
                // can be mounted as anything
                if ( mount_name == 0 )
                {
                    rtn = true;
                    break;
                }

                // mount appears directly after device in /etc/mtab
                mtab >> mount;
                if ( mount.compare(mount_name) == 0 )
                {
                    rtn = true;
                    break;
                }
                else device = mount;
            }
        }
        return rtn;
    }

#ifdef WIN32
    //-------------------------------------------------------------------------------------
    //--------------------  Windows -------------------------------------------------------
    //-------------------------------------------------------------------------------------
    //-------------------------------------------------------------------------------------
    //-------------------------------------------------------------------------------------

    void directory::set( const string& name )
    {
        _name = name;
        dword attr = GetFileAttributes( _name.c_str() );
        if ( attr == INVALID_FILE_ATTRIBUTES )
            _error = GetLastError();
        else
        {
            if ( attr & FILE_ATTRIBUTE_DIRECTORY )
                _error = 0;
            else
                _error = ERROR_PATH_NOT_FOUND;
        }
    }

    //-------------------------------------------------------------------------------------
    string  directory::pwd()
    {
        char buffer[ 256 ];
        GetCurrentDirectory( 256, buffer );
        return string( buffer );
    }

    void directory::get_file_names(list<string>& filesindir)
    {
        string searchpath = full_name("*");
        WIN32_FIND_DATA ffd; 
        HANDLE sh = FindFirstFile(searchpath.c_str(), &ffd);

        if ( sh != INVALID_HANDLE_VALUE )
        {
            do 
            {
                filesindir.push_back(ffd.cFileName);
            } while ( FindNextFile(sh, &ffd) );

            FindClose(sh);
        }
    }



    //-------------------------------------------------------------------------------------
    void partition::get_stats()
    {
        ULARGE_INTEGER   total;
        ULARGE_INTEGER   free;

        if ( 0 != GetDiskFreeSpaceEx( _stat_file_name.c_str(), NULL, &total, &free ) )
        {
            uint freehi = ( free.HighPart << 22 );
            uint freelo = ( free.LowPart >> 10 );
            _space = ( freehi | freelo );
        }
        else
        {
            _space = 0;
            _error =GetLastError();
        }
    }

    //-------------------------------------------------------------------------------------
    bool partition::is_mounted()
    {
        get_stats();
        return ( 0 == _error._error_code );
    }

#else 

    //-------------------------------------------------------------------------------------
    //--------------------  Linux   -------------------------------------------------------
    //-------------------------------------------------------------------------------------
    //-------------------------------------------------------------------------------------

    //-------------------------------------------------------------------------------------
    void directory::set( const string& name  )
    {
        _name = name;
        _error = 0;

        DIR* dir = opendir( _name.c_str() );
        if ( dir == NULL )
            _error = errno;
        else
        {
            //_error = errno;  <--- errno corresponds to last error
            //                      If dir != NULL, errno corresponds to a previous error
            _error = 0;
            closedir( dir );
        }
    }

    //-------------------------------------------------------------------------------------
    string  directory::pwd()
    {
        char buffer[ 256 ];
        getcwd( buffer, 256 );
        return string( buffer );
    }

    void directory::get_file_names(list<string>& filesindir)
    {
        struct dirent *direntp;
        struct dirent de;

        DIR *dir = opendir(_name.c_str());

        //    cout << "Thread: " << getpid() << "opendir() -- " << _name.c_str() << endl;

        if ( dir )
        {
            while ( (readdir_r(dir, &de, &direntp)) == 0 )
            {
                if ( !direntp )
                    break;
                filesindir.push_back(direntp->d_name);

            }

            closedir(dir);
        }
    }

    //-------------------------------------------------------------------------------------
    void partition::get_stats()
    {
        struct statfs stat;
        if ( 0 == statfs( _stat_file_name.c_str(), &stat ) )
        {
            //_space is in 1K increments
            _space = stat.f_bfree * stat.f_bsize;

            DEBUG_INFO( "PT", "optimal block transfer size: " << stat.f_bsize );
            DEBUG_INFO( "PT", "#free blocks: " << stat.f_bfree );
            DEBUG_INFO( "PT", "#free blocks (non-superuser): " << stat.f_bavail );
            DEBUG_INFO( "PT", "#blocks in fs: " << stat.f_blocks );

        }
        else
        {
            _space = 0;
            _error = errno;
        }
    }

    //-------------------------------------------------------------------------------------
    bool partition::is_mounted()
    {
        string cmd = "/bin/mountpoint -q ";
        cmd += _folder_name;
        int rc = system( cmd.c_str() );
        return ( rc == 0 );
    }

    //-------------------------------------------------------------------------------------

#endif //Linux

    // return 0 if error opening, otherwise return greatest mtime
    time_t directory::last_timestamp(time_t passed_timestamp) 
    {
        struct stat statbuf;
        time_t last_timestamp=0;
        int errorcode;

        errorcode = stat(_name.c_str(), &statbuf);
        if ( errorcode ) return 0;    // 0 return from stat means success
        if ( last_timestamp < statbuf.st_mtime ) last_timestamp = statbuf.st_mtime;

        list<string>   filesindir ;
        get_file_names(filesindir);

        for ( list<string>::iterator it=filesindir.begin(); it != filesindir.end(); it++ )
        {
            if ( (it->compare(".") == 0) || 
                 (it->compare("..")) == 0 )
                continue;

            string nextdirstr = full_name((*it).c_str());
            directory nextdir = directory(nextdirstr.c_str());
            time_t dir_last = nextdir.last_timestamp(passed_timestamp);
            if ( last_timestamp < dir_last )
                last_timestamp = dir_last;
        }

        return last_timestamp;
    }

    file_notifier::file_notifier( const char* fn, uint32 mask ) : filename_being_watched(fn), watch_enabled(true), first_wait(true), triggered(false), pending_alert(false), last_timestamp(0)
    {
    }

    file_notifier::~file_notifier()
    {
    }

    void file_notifier::enable_watch()
    {
        DEBUG_INFO( "FS", "enabling watch" );

        struct stat statbuf;
        if ( !stat(filename_being_watched.c_str(), &statbuf) )
        {
            if ( S_ISDIR(statbuf.st_mode) )
            {
                directory watchdir = directory(filename_being_watched.c_str());
                last_timestamp = watchdir.last_timestamp(0);
            }
            else last_timestamp = statbuf.st_mtime;
        }
        watch_enabled = true;
        pending_alert = false;
    }
    void file_notifier::disable_watch()
    {
        DEBUG_INFO( "FS", "disabling watch" );
        watch_enabled = false;
    }

    void file_notifier::trigger()
    {
        triggered = true;
    }

    //-------------------------------------------------------------------------------------
    void file_notifier::wait()
    {
        DEBUG_INFO( "FS", "waiting for file event" );
        struct stat statbuf;
        time_t new_timestamp;
        if ( first_wait )
        {
            first_wait = false;
            if ( stat(filename_being_watched.c_str(), &statbuf) == 0 )
            {
                if ( S_ISDIR(statbuf.st_mode) )
                {
                    directory watchdir = directory(filename_being_watched.c_str());
                    last_timestamp = watchdir.last_timestamp(0);
                }
                else last_timestamp = statbuf.st_mtime;
            }

            // since this is the first wait, we need to sleep to allow the message handler fthread to starts
            thread::sleep(1000);
            pending_alert = true;
            return;
        }
        while ( 1 )
        {
            if ( watch_enabled )
            {
                if ( stat(filename_being_watched.c_str(), &statbuf) == 0 )
                {
                    if ( S_ISDIR(statbuf.st_mode) )
                    {
                        directory watchdir = directory(filename_being_watched.c_str());
                        new_timestamp = watchdir.last_timestamp(last_timestamp);
                    }
                    else new_timestamp = statbuf.st_mtime;

                    if ( (last_timestamp == 0) || (last_timestamp < new_timestamp) )
                    {
                        DEBUG_INFO( "FS", "file changed" );
                        last_timestamp = new_timestamp;
                        break;
                    }
                }
                else
                {
                    // i.e. there was a file before, but it's not there now
                    if ( last_timestamp != 0 )
                    {
                        DEBUG_INFO( "FS", "file deleted" );
                        last_timestamp = 0;
                        break;
                    }
                }
            }
            thread::sleep(1000);
            if ( triggered )
            {
                triggered = false;
                break;
            }

        }
        pending_alert = true;
    }

} // end namespace

