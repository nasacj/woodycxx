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
#include <string.h>
#include "sysipc_command_line.h"

namespace sysipc
{
//-------------------------------------------------------------------------------------
command_line::command_line( int argc, const char* argv[], arglist_entry* alst ) :
   valid( true ), arglist( alst )
{
   parse_command_line( argc, argv );
}

//-------------------------------------------------------------------------------------
command_line::command_line( const char* _cmdline, arglist_entry* alst ) :
   valid( false ), arglist( alst )
{
   if ( argc_object == MAX_COMMAND_LINE_ENTRIES )
       return;

   valid = true;

   string cmdline = _cmdline;
   string::size_type next_quote, next_space, separator_pos, stake_pos;
   
   stake_pos = 0;
   separator_pos = 0;
   argc_object = 0;
   
   while ( stake_pos < cmdline.length() )
   {
      next_quote = cmdline.find( "\"", stake_pos );
      next_space = cmdline.find( " ",  stake_pos );
   
      separator_pos = string::npos;
      if ( next_quote < next_space )
      {
         next_quote = cmdline.find( "\"", next_quote + 1 );
         
         if ( next_quote != string::npos )
            separator_pos = cmdline.find( " ", next_quote + 1 );
      }
      else
      {
         separator_pos = next_space;
      }
      
      if ( separator_pos == string::npos )
         separator_pos = cmdline.length();

      // Remove the quotes from argv[0]
      if ( argc_object == 0 )
         tokens.push_back( cmdline.substr( stake_pos + 1, separator_pos - stake_pos - 1 ) );
      else
         tokens.push_back( cmdline.substr( stake_pos, separator_pos - stake_pos ) );
      
      char* x = ( char* ) tokens.back().c_str();
      argv_object.add_argument( x );
      //argv_object.add_argument( ( char* ) tokens.back().c_str() );
      stake_pos = separator_pos + 1;
      argc_object++;
   }
}

//-------------------------------------------------------------------------------------
void command_line::parse_command_line( int argc, const char* argv[])
{
   argc_object = argc;
   
   for ( int i = 1; i < argc; i++ )
   {
      if ( '-' == *( argv[ i ] )  )
      {
         const char* opt = argv[ i ]+1; 
         arglist_entry* entry = find_entry( opt );
         if ( entry )
         {
            entry->valid = true;
            switch ( entry->arg_type )
            {
               case arglist_entry::int_opt:
                  i++;
                  if ( argv[i] )
                     entry->ival = atoi( argv[ i ] );
                  break;

               case arglist_entry::alpha_opt:
                  entry->sval = "";
                  if ( i+1 < argc && argv[ i+1 ][0] != '-' )
                  {
                     i++;
                     entry->sval = argv[ i ];
                  }
                  break;

               case arglist_entry::alpha_opt_multiple:
                  {
                     i++;
                     if ( i < argc )
                     {
                        arglist_entry_multiple* lval = new arglist_entry_multiple;
                        lval->sval = argv[ i ];
                        if ( entry->lval )
                        {
                           // add to end of list
                           arglist_entry_multiple* list_entry = entry->lval;
                           while ( list_entry->next() )                 
                              list_entry = list_entry->next();
                           list_entry->_next = lval;
                        }
                        else
                           entry->lval = lval;
                     }
                  }
                  break;
            }
         }
         else
         {
            cerr << "Error: invalid option: " << opt << endl;
            valid = false;
            break;
         }
      }
      else
      {
         /*-----------------------------------------------------------------
          * If not an option, all remaing arguments considered files
          *---------------------------------------------------------------*/
         read_file_names( argc - i, ( const char** ) &argv[ i ] );
         i = argc;
      }
   }

   // is there a required file name option???
   if ( find_entry_type( arglist_entry::req_fname ) )
   {
      if ( file_names.size() != 1 )
         valid = false;
   }

   if ( find_entry_type( arglist_entry::req_fnames )  )
   {
         if ( file_names.size() == 0 )
            valid = false;
   }

   switch ( file_names.size() )
   {
       case 0:
           break;

       case 1:
           if ( !( find_entry_type( arglist_entry::fname ) || find_entry_type( arglist_entry::fnames ) ) )
               valid = false;
           break;

       default:
           if ( !find_entry_type( arglist_entry::fnames ) )
               valid = false;
           break;
   }
}

//-------------------------------------------------------------------------------------
command_line::~command_line()
{
   for ( uint i = 0; i < file_names.size(); i++ )
      delete file_names[ i ];
}


//-------------------------------------------------------------------------------------
arglist_entry* command_line::find_entry_type( int type )
{
   arglist_entry* entry = arglist;

   while ( entry->arg_type != arglist_entry::end_of_list )
   {
      if ( entry->arg_type == type )
         break;
      entry++;
   }
   if ( entry->arg_type == arglist_entry::end_of_list )
      entry = 0;

   return entry;
}

//-------------------------------------------------------------------------------------
arglist_entry* command_line::find_entry( const char* opt )
{
   bool done            = false;
   arglist_entry* entry = arglist;

   while ( !done )
   {
      switch ( entry->arg_type )
      {
         case arglist_entry::whatever_opt:
            if ( 0 ==
             strncmp( entry->option_chars, opt, strlen(entry->option_chars) ) )
               { done = true; }
            else
               { entry++; }
            break;

         case arglist_entry::opt:
         case arglist_entry::int_opt:
         case arglist_entry::alpha_opt:
         case arglist_entry::alpha_opt_multiple:
            if ( 0 == strcmp( entry->option_chars, opt ) )
               done = true;
            else
               entry++;
            break;

         case arglist_entry::end_of_list:
            entry = 0;
            done = true;
            break;

         default:
            entry++;
            break;
      }
   }
   return entry;
}

//-------------------------------------------------------------------------------------
void command_line::show_options()
{
   arglist_entry*    arg = arglist;
   bool  done = false;

   string fname_option("");
   string fname_descr;
   bool  options = false;

   while ( !done )
   {
      switch ( arg->arg_type )
      {
         case arglist_entry::whatever_opt:
         case arglist_entry::opt:
         case arglist_entry::int_opt:
         case arglist_entry::alpha_opt:
         case arglist_entry::alpha_opt_multiple:
            options = true;
            break;

         case arglist_entry::req_fname:
            fname_option = "fname";
            fname_descr = arg->desc;
            break;

         case arglist_entry::req_fnames:
            fname_option = "fname fname ...";
            fname_descr = arg->desc;
            break;

         case arglist_entry::fname:
            fname_option = "<";
            fname_option += "fname";
            fname_option += ">";
            fname_descr = arg->desc;
            break;

         case arglist_entry::fnames:
            fname_option = "<";
            fname_option += "fname fname ...";
            fname_option += ">";
            fname_descr = arg->desc;
            break;

         case arglist_entry::end_of_list:
            done = true;
            break;
      }
      arg++;
   }

   if ( options )
      cout << "[ options ] ";

   if ( fname_option.length() > 0 )
      cout << fname_option;

   cout << "" << endl;

   if ( fname_option.length() > 0 )
   {
      cout << "   fname: " << fname_descr << "" << endl ;
   }

   if ( options )
   {
      cout << "   Options:" << endl;
      arg = arglist;
      done = false;
      while ( !done )
      {
         switch ( arg->arg_type )
         {
            case arglist_entry::opt:
               cout << "      -" << arg->option_chars << arg->desc << endl;
               break;
      
            case arglist_entry::whatever_opt:
            case arglist_entry::int_opt:
               cout << "      -" << arg->option_chars << arg->desc << endl;
               break;
      
            case arglist_entry::alpha_opt:
            case arglist_entry::alpha_opt_multiple:
               cout << "      -" << arg->option_chars << arg->desc << endl;
               break;
      
            case arglist_entry::end_of_list:
               done = true;
               break;
         }
         arg++;
      }
      cout << endl;
   }
   cout << endl;
}

//-------------------------------------------------------------------------------------
bool command_line::exists_option( const char* opt )
{
   arglist_entry* entry = find_entry( opt );
   return ( 0 != entry ) && entry->valid;
}

//-------------------------------------------------------------------------------------
const char* command_line::exists_alpha_option( const char* opt )
{
   const char* val = 0;

   arglist_entry* entry = find_entry( opt );
   if ( entry && ( entry->arg_type == arglist_entry::alpha_opt  ) )
      val = entry->sval;

   return val;
}

//-------------------------------------------------------------------------------------
const char* command_line::get_alpha_option( const char* opt, const char* def_value )
{
    const char* val = def_value;

    arglist_entry* entry = find_entry( opt );
    if ( entry && ( entry->arg_type == arglist_entry::alpha_opt  ) && entry->sval )
       val = entry->sval;

    return val;
}


//-------------------------------------------------------------------------------------
const arglist_entry_multiple* command_line::exists_alpha_option_multiple( const char* opt )
{
   arglist_entry_multiple* rslt  = 0;
   arglist_entry* entry = find_entry( opt );

   if ( entry && ( entry->arg_type == arglist_entry::alpha_opt_multiple  ) )
      rslt = entry->lval;
   return rslt;
}

//-------------------------------------------------------------------------------------
int command_line::get_integer_option( const char* opt, int def_value )
{
   int val = def_value;

   arglist_entry* entry = find_entry( opt );
   if ( entry && ( entry->arg_type == arglist_entry::int_opt  ) && entry->valid )
      val = entry->ival;

   return val;
}

//-------------------------------------------------------------------------------------
const char* command_line::get_fname( uint n )
{
   if ( n < file_names.size() )
      return file_names[ n ]->c_str();
   else
      return 0;
}

//-------------------------------------------------------------------------------------
void command_line::read_file_names( uint num_files, const char** names )
{
   if ( num_files > 0 )
   {
      if ( num_files == 1 )
      {
         const char* fn = names[ 0 ];
         // is the a file of file names???
         if ( *fn  == '@' )
         {
            // try to open file...( skip over @ )
            //input_file  response_file( fn + 1 );
            ifstream    strm( fn + 1 );
            while ( !strm.eof() )
            {
               char buffer[ 256 ];
               strm.getline( buffer, 255 );
               buffer[ 255 ] = 0;
               if ( strm.gcount() > 1 )
               {
                  string*  name = new string( buffer );
                  file_names.push_back( name );
               }
            }
         }
         else
         {
            // just a regular file
            string* name = new string( names[ 0 ] );
            file_names.push_back( name );
         }
      }
      else
      {
         for ( uint i = 0; i < num_files; i++ )
         {
            string*  name = new string( names[ i ] );
            file_names.push_back( name );
         }
      }
   }
}

//-------------------------------------------------------------------------------------
void command_line_arguments::add_argument( char* arg )
{
   _pointers.push_back( arg );
}

}

