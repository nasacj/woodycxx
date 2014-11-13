/* -------------------------------------------------------
 *
 * Licensed Materials - Property of IBM.
 * (C) Copyright IBM Corporation 2001, 2004
 * All Rights Reserved.
 *
 * US Government Users Restricted Rights -
 * Use, duplication or disclosure restricted by
 * GSA ADP Schedule Contract with IBM Corporation.
 *
 * Filename : sim_streams
 *
 * Originator : Mark Rinaldi
 *
 * Purpose  :
 *
 *
 * ------------------------------------------------------- */
#include "sysipc_exceptions.h"
#include <iomanip>
#include <string.h>

namespace sysipc
{
/*--------------------------------------------------------------------------
 * operator<<( ostream&, const location& )
 *------------------------------------------------------------------------*/
ostream& operator<<( ostream& strm, const location& loc )
{
   if ( loc.file_name )
   {
        const char* fname = strrchr( loc.file_name, '/' );
        if ( fname )
            fname++;
        else 
            fname = loc.file_name;

        strm << setw(30) << left << fname << ", " << setw(4) << loc.line_number << "  ";
   }
   return strm;
}


/*--------------------------------------------------------------------------
 * operator<<( ostream&, const strem_message& )
 *------------------------------------------------------------------------*/
ostream& operator<<( ostream& strm, const stream_message& msg )
{
    strm << msg.message << " " << msg.loc;
    return strm;
}

/*--------------------------------------------------------------------------
 * operator<<( ostream&, const exception& )
 *------------------------------------------------------------------------*/
ostream& operator<<( ostream& strm, const exception& exc )
{
    strm << exc._msg;
    return strm;
}

}


