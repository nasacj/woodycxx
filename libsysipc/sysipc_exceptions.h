/* -------------------------------------------------------
 *
 * Licensed Materials - Property of IBM.
 * (C) Copyright IBM Corporation 2004
 * All Rights Reserved.
 *
 * US Government Users Restricted Rights -
 * Use, duplication or disclosure restricted by
 * GSA ADP Schedule Contract with IBM Corporation.
 *
 * Filename : sim_basictypes_t.h
 *
 * Originator : Mark Rinaldi
 *
 * Purpose  : Generic Types used in Simulator
 *
 *
 * ------------------------------------------------------- */
#ifndef _SYSIPC_EXCEPTIONS_H_
#define _SYSIPC_EXCEPTIONS_H_
#include "sysipc.h"
#include <sstream>    
#include <ostream>

namespace sysipc
{
/*--------------------------------------------------------------------------
 * location
 *------------------------------------------------------------------------*/
struct location
{
   const char* file_name;
   uint        line_number;

   location( const char* fn = 0, uint ln = 0 ) : 
      file_name( fn ), line_number( ln ){}
};
ostream& operator<<( ostream&, const location& );


/*--------------------------------------------------------------------------
 * File and line number info
 *------------------------------------------------------------------------*/
#define __SYS_FILE  __FILE__
#define __SYS_LINE  __LINE__
#define LOCATION location( __SYS_FILE, __SYS_LINE )

/*--------------------------------------------------------------------------
 * stream message
 *------------------------------------------------------------------------*/
struct stream_message
{
   string      message;
   location    loc;

   stream_message( const string& str, const char* fn = 0, uint ln = 0 ) : 
      message( str ), loc( fn, ln ){}

   stream_message( const char* fn = 0, uint ln = 0 ) : 
      loc( fn, ln ){}
};
ostream& operator<<( ostream&, const stream_message& );

/*--------------------------------------------------------------------------
 * struct exception
 *------------------------------------------------------------------------*/
struct exception
{
   stream_message _msg;

public:
   exception( const string& in_str, const char* fn = 0, uint ln = 0 ) : 
      _msg( in_str, fn, ln ){}

   const string& str() const { return _msg.message; }
   const stream_message& msg() const { return _msg; }
   string& str() { return _msg.message; }
};
ostream& operator<<( ostream&, const exception& exc );

#define SYS_EXCEPTION_IF(cond,m)\
if ( cond )\
{\
   ostringstream msg( " " );\
   msg << m << ends;\
   sysipc::exception exc( msg.str(), __SYS_FILE, __SYS_LINE );\
   throw exc;\
}

#define SYS_EXCEPTION(m)\
{\
   ostringstream msg( " " );\
   msg << m << ends;\
   sysipc::exception exc( msg.str(), __SYS_FILE, __SYS_LINE );\
   throw exc;\
}


#define SYS_ASSERT(c)\
if ( !( c ) )\
{\
    cout << "assertion failed: ";\
    cout << location( __SYS_FILE, __SYS_LINE ) << endl;\
    abort();\
}

}

#endif 
