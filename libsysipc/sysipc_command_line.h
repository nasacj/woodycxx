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
#ifndef _SYSIPC_COMMAND_LINE_H_
#define _SYSIPC_COMMAND_LINE_H_

#include "sysipc.h"
#include <fstream>

/// \ingroup sysipc
/// \file sysipc_command_line.h
/// \brief command line support
/// command_line::command_line 
///    For the simple case, the spaces in the input string are our delimiters, and
///    will become the 'separator positions'. Tokens are extracted between the
///    stake position and the separator posistion.
///  
///    For the advanced case, we need to consider quotes, which may occur if there are
///    spaces in the directory names.
///  
///    If a quote is detected before the next space, then the next separator position
///    is the space that follows the next quote. For the case where there is no 'end'
///    quote, treat the rest of the line as one token.

namespace sysipc
{

//------------------------------------------------------------------------------------------------------------------
/// \ingroup sysipc
class command_line_arguments
{
   vector< char* >   _pointers;

public:
   void     add_argument( char* );
   uint     size(){ return static_cast<uint>( _pointers.size() );}
   char**   arguments(){ return &_pointers[ 0 ]; }
};


//------------------------------------------------------------------------------------------------------------------
/// \ingroup sysipc
struct arglist_entry_multiple
{
   const char*             sval;
   arglist_entry_multiple* _next;
   arglist_entry_multiple() : sval( 0 ), _next( 0 ){}
   arglist_entry_multiple* next(){ return _next;  }
   const arglist_entry_multiple* next() const { return _next;  }
};

//------------------------------------------------------------------------------------------------------------------
/// \ingroup sysipc
struct arglist_entry
{
   enum argtype { opt, whatever_opt, int_opt, alpha_opt, alpha_opt_multiple, fname, fnames, req_fname, req_fnames, end_of_list };
   enum file_opts { required, not_required, none }; 
   bool           valid;
   int            arg_type;
   const char*    option_chars;
   const char*    desc;
   union
   {
      const char* sval;
      int         ival;
      arglist_entry_multiple* lval;
   };
};

#define CMDL_BEGIN_OPTION_LIST \
sysipc::arglist_entry arglist[]={

#define CMDL_END_OPTION_LIST { false, sysipc::arglist_entry::end_of_list, 0, 0, {0} }};
#define CMDL_ARG_OPTION( optchars , desc ) { false, sysipc::arglist_entry::opt, optchars, desc, {0} },
#define CMDL_ARG_WHATEVER_OPTION( optchars , desc ) { false, sysipc::arglist_entry::whatever_opt, optchars, desc, {0} },
#define CMDL_ARG_INT_OPTION( optchars, desc ) { false, sysipc::arglist_entry::int_opt, optchars, desc, {0} },
#define CMDL_ARG_ALPHA_OPTION( optchars, desc ) { false, sysipc::arglist_entry::alpha_opt, optchars, desc, {0} },
#define CMDL_ARG_ALPHA_OPTION_MULTIPLE( optchars, desc ) { false, sysipc::arglist_entry::alpha_opt_multiple, optchars, desc, {0} },
#define CMDL_ARG_FNAME_OPTION( desc ) { false, sysipc::arglist_entry::fname, 0, desc, {0} },
#define CMDL_ARG_REQ_FNAME_OPTION( desc ) { false, sysipc::arglist_entry::req_fname, 0, desc, {0} },
#define CMDL_ARG_FNAMES_OPTION( desc ) { false, sysipc::arglist_entry::fnames, 0, desc, {0} },
#define CMDL_ARG_REQ_FNAMES_OPTION( desc ) { false, sysipc::arglist_entry::req_fnames, 0, desc, {0} },


const uint MAX_COMMAND_LINE_ENTRIES = 64;

//------------------------------------------------------------------------------------------------------------------
/// \ingroup sysipc
class command_line
{
   bool                    valid;
   vector< string* >       file_names;

   command_line_arguments  argv_object;
   vector< string >        tokens;
   uint                    argc_object;

   arglist_entry*          arglist;
   arglist_entry* find_entry( const char* opt );
   arglist_entry* find_entry_type( int type );
   string  commands  ;

   void read_file_names( uint nfiles, const char** names );

public:
   void show_options();
   command_line(const char *, arglist_entry* alst );
   command_line( int argc, const char* argv[], arglist_entry* alst );
   ~command_line();
   void parse_command_line( int argc, const char* argv[] );
   bool exists_option( const char* opt );
   const char* exists_alpha_option( const char* opt );
   const arglist_entry_multiple* exists_alpha_option_multiple( const char* opt );
   int get_integer_option( const char* opt, int def_value );
   const char* get_alpha_option( const char* opt, const char* def_value );
   const char* get_fname( uint n = 0 );
   uint num_files()  { return static_cast<uint>( file_names.size() ); }
   bool is_valid()   { return valid; }
   int get_argc()    { return argc_object; }
   char** get_argv() { return argv_object.arguments(); }
};

}

#endif
