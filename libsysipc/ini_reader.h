/******************************************************************************
 *
 *  Licensed Materials - Property of IBM.
 *
 *  (C) Copyright IBM Corporation 2011
 *
 *  All Rights Reserved.
 *
 *  US Government Users Restricted Rights -
 *  Use, duplication or disclosure restricted by
 *  GSA ADP Schedule Contract with IBM Corporation.
 *
 *****************************************************************************/
#ifndef _ini_reader_h
#define _ini_reader_h

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>

using namespace std;

namespace ini_file
{

struct ini_file_entry 
{
    string key;
    string value;
};

// derive a class from this base class and implement these functions
class ini_handler 
{
public:
    virtual void add_group(const string& group) = 0;
    virtual void add_entry(const ini_file_entry& entry) = 0;
};

class ini_file_reader
{
    ini_handler& handler;
public:
    ini_file_reader(ini_handler& hdlr) : handler(hdlr)  {}

    static string trim(const string& instr)
    {
        const string& trimchars = string(" \t");
        size_t start = instr.find_first_not_of(trimchars);
        if (start == string::npos)
            return "";

        size_t end = instr.find_last_not_of(trimchars);
        size_t len = end - start + 1;

        return instr.substr(start, len);
    }

    //--------------------------------------------------------------------------------------------
    int read_file( istream& strm )
    {
        ini_file_entry ent;
        string bit_bucket;
        string line_str;

        while ( strm.good() )
        {
            getline( strm, line_str );

            // ignore blank lines
            if ( line_str.length() == 0 )
                continue;

            // ignore whitespace only lines
            size_t strt = line_str.find_first_not_of(' ');
            if ( strt == string::npos )
                continue;

            // ignore comment lines
            if ( line_str.at( strt ) == '#' || 
                 line_str.at( strt ) == ';' )
                continue;

            if ( line_str.at( strt ) == '[' ) 
            {
                size_t fnd = line_str.find_first_of(']');
                if ( fnd != string::npos )
                {
                    string group = trim(line_str.substr(strt+1, fnd - strt - 1));
                    handler.add_group(group);
                    // cout << "group =" << group << endl;
                }
            }
            else
            {
                size_t fnd = line_str.find_first_of('=');
                if ( fnd != string::npos )
                {
                    ent.key = trim(line_str.substr(0, fnd));
                    ent.value = trim(line_str.substr(fnd+1, line_str.length() )); 
                    handler.add_entry(ent);
                    // cout << ent.key << "=" << ent.value << endl;
                }
            }
        }
        return 0;
    }

};

};


#endif
