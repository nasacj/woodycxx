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
#include "sysipc_security_wrap.h"
#include <libcryptowrap.h>
#include <string.h>
#include <iomanip>
#include <fstream>
#include "syslog.h"


namespace sysipc
{

int security_wrap::chomp_cmd(const char** cmd, unsigned int& exec_mode)
{
    const char*   cmdline = *cmd;
    int     cmd_len = strlen(cmdline);
    const char*   p_start = cmdline;
    const char*   p_end   = cmdline + cmd_len - 1;
    bool    backgound_mode_found = false;

    while(p_start <= p_end){
        if(*p_start != ' '){
            break;
        }
        else{
            p_start++;
        }
    }
    
    if(p_start > p_end){
        return -1;
    }

    while(p_start <= p_end){
        if(*p_end != ' '){
            if(*p_end == '&' && !backgound_mode_found){
                backgound_mode_found = true;
                p_end--;
            }
            else{
                break;
            }
        }
        else{
            p_end--;
        }
    }

    if(p_start > p_end){
        return -1;
    }

    cmd = &p_start;
    exec_mode = (backgound_mode_found == true) ? stfu::sExec::MODE_DAEMON : stfu::sExec::MODE_WAIT;
    return (p_end - p_start + 1);
}

bool security_wrap::parse_commandline(const char* cmdline, vector<string>& para_list, vector<redirection>& redirect_list, unsigned int& exec_mode)
{
    int cmd_len = chomp_cmd(&cmdline, exec_mode);
    if(cmd_len < 0){
        syslog(LOG_WARNING,"ERROR: security_wrap::exec: chomp command line failed");
        return false;
    }

    char* para_buf = new char[SYSTEM_MAX_PARA_BUF_LEN];
    char* para = para_buf;
    char* const para_end = para_buf + SYSTEM_MAX_PARA_BUF_LEN - 1;
    const char* p_cmd = cmdline;
    const char* const p_cmd_end = cmdline + cmd_len;
    bool quotes_flag = false;
    bool redirect_flag = false;
    bool rtn = true;
    int loop = 0;
    redirection redirect;


    while( p_cmd <= p_cmd_end && rtn){
        //the below check is to avoid the function goes into deal loop
        loop++;
        if(loop > cmd_len + 1){
            syslog(LOG_WARNING,"ERROR: security_wrap::exec: the loop goes into deal loop, force to break");
            rtn = false;
            break;
        }

        if(*p_cmd == ' ' || p_cmd == p_cmd_end){
            if(!quotes_flag){
                if(para != para_buf){
                    *para = '\0';
                    if(redirect_flag){
                        redirect.output = para_buf;
                        redirect_list.push_back(redirect);
                        redirect_flag = false;
                    }
                    else{
                        para_list.push_back(para_buf);
                    }
                    para = para_buf;
                }
                p_cmd++;
                continue;
            }
        }
        else if(*p_cmd == '"'){
            if(!quotes_flag){
                quotes_flag = true;
                if(para != para_buf){
                    syslog(LOG_WARNING,"ERROR: security_wrap::exec: quotes appears in the middle of parameter");
                    rtn = false;
                }
                else{
                    //skip the beginning quotes
                    p_cmd++;
                }
            }
            else{
                quotes_flag = false;
                if( para == para_buf ){
                    syslog(LOG_WARNING,"ERROR: security_wrap::exec: no parameter between quotes");
                    rtn = false;
                    continue;
                }
                *para = '\0';
                if(redirect_flag){
                    redirect.output = para_buf;
                    redirect_list.push_back(redirect);
                    redirect_flag = false;
                }
                else{
                    para_list.push_back(para_buf);
                }
                para = para_buf;
                p_cmd++;
            }
            continue;
        }
        else if(*p_cmd == '\\'){
            char escape = *(p_cmd+1);
            if(!quotes_flag){
                //escape character should be included in quotes
                syslog(LOG_WARNING,"ERROR: security_wrap::exec: escape char appear out of quotes");
                rtn = false;
                continue;
            }
            switch(escape){
                case '"':
                case '\\':
                    p_cmd++;
                    break;
                default:
                    syslog(LOG_WARNING,"ERROR: security_wrap::exec: unknown escape character");
                    rtn = false;
            }
            if(!rtn){
                continue;
            }
        }
        else if(*p_cmd == '>'){
            if(!quotes_flag){
                if(redirect_flag){
                    syslog(LOG_WARNING,"ERROR: security_wrap::exec: redirection parse error");
                    rtn = false;
                    continue;
                }
                redirect_flag = true;
                if(para ==  para_buf){
                    if(para_list.empty()){
                        redirect.input = "1";
                    }
                    else{
                        string last_para = para_list.back();
                        if(last_para.compare("1")==0 || last_para.compare("2")==0){
                            //last para is actually a direction input
                            redirect.input = last_para;
                            para_list.pop_back();
                        }
                        else{
                            redirect.input = "1";
                        }
                    }
                }
                else{
                    *para = '\0';
                    if(strcmp(para_buf,"1")==0 || strcmp(para_buf,"2")==0){
                        redirect.input = para_buf;
                    }
                    else{
                        redirect.input = "1";
                        para_list.push_back(para_buf);
                    }
                }
                if(*(p_cmd+1) == '>'){
                    redirect.attr = APPEND;
                    p_cmd += 2;
                }
                else{
                    redirect.attr = WRITE;
                    p_cmd++;
                }
                para = para_buf;
                continue;
            }
        }
        else if(*p_cmd == '<'){
            if(!quotes_flag){
                syslog(LOG_WARNING,"ERROR: security_wrap::exec: redirect input is not supported");
                rtn = false;
            }
        }
        else if(*p_cmd == '|'){
            if(!quotes_flag){
                syslog(LOG_WARNING,"ERROR: security_wrap::exec: pipe is not supported");
                rtn = false;
            }
        }

        *para++ = *p_cmd++;
        if(para >= para_end){
            syslog(LOG_WARNING,"ERROR: security_wrap::exec: parameter is too long");
            rtn = false;
        }
    }

    delete [] para_buf;
    para_buf = NULL;
    
    return rtn;
}

int security_wrap::parse_redirection(vector<redirection>& redirect_list, redirection_file& stdout_file, redirection_file& stderr_file)
{
    int flag = 0;
    bool stderr_file_set = false;
    bool stdout_file_set = false;

    for(vector<redirection>::iterator it = redirect_list.begin(); it != redirect_list.end(); it++){
        string input = it->input;
        string output = it->output;
        unsigned int attr = it->attr;

        if(attr != WRITE && attr != APPEND){
            syslog(LOG_WARNING,"ERROR: security_wrap::exec: unsupported redirection operation");
            flag = -1;
            break;
        }

        if(input.compare("1") == 0){
            if(output.compare("&2") == 0){
                flag |= redirect_flag_mrg2stderr;
            }
            else if(output.compare("/dev/null") == 0){
                flag |= redirect_flag_ign_stdout;
            }
            else{
                flag |= redirect_flag_set_stdout;
                if(!stdout_file_set){
                    stdout_file_set = true;
                    stdout_file.name = output;
                    stdout_file.attr = attr;
                }
                else{
                    flag = -1;
                    break;
                }
            }
        }
        else if(input.compare("2") == 0){
            if(output.compare("&1") == 0){
                flag |= redirect_flag_mrg2stdout;
            }
            else if(output.compare("/dev/null") == 0){
                flag |= redirect_flag_ign_stderr;
            }
            else{
                flag |= redirect_flag_set_stderr;
                if(!stderr_file_set){
                    stderr_file_set = true;
                    stderr_file.name = output;
                    stderr_file.attr = attr;
                }
                else{
                    flag = -1;
                    break;
                }
            }
        }
        else{
            syslog(LOG_WARNING,"ERROR: security_wrap::exec: unknown redirection input");
            flag = -1;
            break;
        }
    }

    return flag;
}

int security_wrap::redirect_2_file(vector<unsigned char>& bytes, redirection_file& file)
{
    const char* file_name = file.name.c_str();
    ios_base::openmode mode;
    if( file.attr == WRITE ){
        mode = ios_base::out;
    }
    else if( file.attr == APPEND ){
        mode = ios_base::app;
    }
    else{
        return -1;
    }
    
    ofstream output_file;
    output_file.open(file_name, mode);
    if(!output_file.good()){
        return -1;
    }
    if(bytes.size() > 0){
        output_file.write((const char*)bytes.data(), bytes.size());
    }
    output_file.close();
    return 0;
}

int security_wrap::system(const char* commandline)
{
    stfu::sExec Runner;
    vector<string> para_list;
    vector<redirection> redirect_list;
    unsigned int exec_mode;
    bool has_set_program = false;
    int result = 0;
    int rdct_parse_flag = 0;
    redirection_file stdout_file;
    redirection_file stderr_file;
    

    if( !parse_commandline( commandline, para_list, redirect_list, exec_mode ) ){
        return -1;
    }


    Runner.setMode( exec_mode );

    if(para_list.empty()){
        return -1;
    }

    for(vector<string>::iterator it = para_list.begin(); it != para_list.end(); it++){
        //syslog(LOG_WARNING,"INFO: security_wrap::system: parsed paramter --> %s\n", *it);
        if(has_set_program){
            Runner.addArgument(*it);
        }
        else{
            has_set_program = true;
            if(!Runner.setProgram(*it)){
                return -1;
            }
        }
    }

    if(!redirect_list.empty()){
        rdct_parse_flag = parse_redirection(redirect_list, stdout_file, stderr_file);
        syslog(LOG_WARNING,"INFO: security_wrap::system: parsed redirection flag --> %d\n", rdct_parse_flag);
        if(rdct_parse_flag < 0){
            return -1;
        }
        if( rdct_parse_flag & redirect_flag_set_stdout){
            Runner.setFlag( stfu::sExec::FLAG_BUFFERSTDOUT );
            if( rdct_parse_flag & redirect_flag_mrg2stdout ){
                Runner.setFlag( stfu::sExec::FLAG_BUFFERSTDERR );
            }
        }
        if( rdct_parse_flag & redirect_flag_set_stderr ){
            Runner.setFlag( stfu::sExec::FLAG_BUFFERSTDERR );
            if( rdct_parse_flag & redirect_flag_mrg2stderr ){
                Runner.setFlag( stfu::sExec::FLAG_BUFFERSTDOUT );
            }
        }
    }


    result = Runner.Execute();

    if(!redirect_list.empty()){
        vector<unsigned char> bytes;
        if( rdct_parse_flag & redirect_flag_set_stdout){
            bytes.clear();
            Runner.collectStdout(bytes);
            redirect_2_file(bytes, stdout_file);
            if( rdct_parse_flag & redirect_flag_mrg2stdout ){
                bytes.clear();
                Runner.collectStderr(bytes);
                redirect_2_file(bytes, stdout_file);
            }
        }
        if( rdct_parse_flag & redirect_flag_set_stderr ){
            bytes.clear();
            Runner.collectStderr(bytes);
            redirect_2_file(bytes, stdout_file);
            if( rdct_parse_flag & redirect_flag_mrg2stderr ){
                bytes.clear();
                Runner.collectStdout(bytes);
                redirect_2_file(bytes, stdout_file);
            }
        }
    }


    return result;
}


}

