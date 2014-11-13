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
#ifndef __SYSIPC_SECURITY_WRAP
#define __SYSIPC_SECURITY_WRAP

#include <string>
#include <vector>
 //------------------------------------------------------------------------------------------
 /// \ingroup sysipc
 /// \brief secure functions to replace system() routine
using namespace std;

namespace sysipc
{

 class security_wrap
 {
 private:
     enum redirection_attr
     {
         READ = 0,
         WRITE,
         APPEND
     };
 
     struct redirection
     {
         string     input;
         string     output;
         unsigned int    attr;
         redirection()
         {
             input.clear();
             output.clear();
             attr = 0;
         }
     };
 
     struct redirection_file
     {
         string          name;
         unsigned int    attr;
         redirection_file()
         {
             name.clear();
             attr = 0;
         }
     };
 
     static const int SYSTEM_MAX_PARA_BUF_LEN = 128;
     static const int redirect_flag_ign_stdout = 0x01;
     static const int redirect_flag_ign_stderr = 0x02;
     static const int redirect_flag_set_stdout = 0x04;
     static const int redirect_flag_set_stderr = 0x08;
     static const int redirect_flag_mrg2stdout = 0x10;
     static const int redirect_flag_mrg2stderr = 0x20;
 
     //declare the constructor to private to avoid object to be allocated.
     security_wrap(void) {}
     static int chomp_cmd(const char** cmd, unsigned int& exec_mode);
     static bool parse_commandline(const char* cmdline, vector<string>& para_list, vector<redirection>& redirect_list, unsigned int& exec_mode);
     static int parse_redirection(vector<redirection>& redirect_list, redirection_file& stdout_file, redirection_file& stderr_file);
     static int redirect_2_file(vector<unsigned char>& bytes, redirection_file& file);
 public:
     static int system(const char* commandline);
};

}
#endif

