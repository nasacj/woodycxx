/******************************************************************************
 * 
 *  Licensed Materials - Property of IBM.
 * 
 *  (C) Copyright IBM Corporation 2007
 * 
 *  All Rights Reserved.
 * 
 *  US Government Users Restricted Rights -
 *  Use, duplication or disclosure restricted by
 *  GSA ADP Schedule Contract with IBM Corporation.
 * 
 *****************************************************************************/
#ifndef __SYSIPC_DYNALIB_H
#define __SYSIPC_DYNALIB_H

#include "sysipc.h"

/// \ingroup sysipc
/// \file sysipc_dynalib.h
/// \brief Thread support

#ifdef WIN32
#include <windows.h>
#define DYNALIB_HANDLE HINSTANCE
#else
#define DYNALIB_HANDLE void*
#endif


namespace sysipc
{

/// \ingroup sysipc
/// \brief Thread handler
class dynamic_library
{
    DYNALIB_HANDLE          library_handle;
    sysipc::os_error        error_code;
    bool                    loaded;
	
public:
    dynamic_library();

    /// \brief Loads the dynamic plugin library
    bool dlopen( const char* name, int flags = 0 );
    void dlclose();

    void* dlsym( const char* sym_name );
    bool is_loaded() const { return loaded;}
};
}
#endif
