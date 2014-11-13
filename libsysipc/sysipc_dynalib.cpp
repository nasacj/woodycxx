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
#include "sysipc_dynalib.h"

#ifdef WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#endif

namespace sysipc
{

//-----------------------------------------------------------------------------------------------------------
dynamic_library::dynamic_library() :
    library_handle( 0 )
{
    loaded = false;
}

#ifdef WIN32
//-----------------------------------------------------------------------------------------------------------
bool dynamic_library::dlopen( const char* name, int flags )
{
    if ( !is_loaded() )
    {
        string fn( name );
        fn += ".dll";
        // don't load the library twice
        library_handle = LoadLibrary( fn.c_str() );
        if ( library_handle != 0 )
            loaded = true;
        else
            error_code = GetLastError();
    }
    return is_loaded();
}

//-----------------------------------------------------------------------------------------------------------
void* dynamic_library::dlsym( const char* sym_name )
{
    void* sym = 0;

    if ( is_loaded() )
        sym = GetProcAddress( library_handle, sym_name );
    return sym;
}

//-----------------------------------------------------------------------------------------------------------
void dynamic_library::dlclose()
{
    if ( is_loaded()  )
    {
        FreeLibrary( library_handle );
        library_handle = 0;
        error_code = GetLastError();
        loaded = false;
    }
}

#else

//-----------------------------------------------------------------------------------------------------------
bool dynamic_library::dlopen( const char* name, int flags )
{
    // don't load the library twice
    if ( !is_loaded() )
    {
        string fn( name );
        fn += ".so";
        // open library, need RTLD_GLOBAL to ensure RTTI works properly
        library_handle = ::dlopen( fn.c_str(), flags );
        if ( library_handle != 0 )
            loaded = true;
        else
            error_code = errno;
    }
    return is_loaded();
}

//-------------------------------------------------------------------------------------
void* dynamic_library::dlsym( const char* sym_name )
{
    void* sym = 0;
    if ( is_loaded() )
        sym = ::dlsym( library_handle, const_cast<char*>( sym_name ) );
    return sym;
}

//-------------------------------------------------------------------------------------
void dynamic_library::dlclose()
{
    if ( is_loaded() )
    {
        ::dlclose( library_handle );
        library_handle = 0;
        error_code = errno;
        loaded = false;
    }
}

#endif

} // end namespace


