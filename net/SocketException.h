/*
Copyright (c) 2014-2015 by NASa Qian <nasacj@nasacj.net>
This file is part of the woodycxx library.

This software is distributed under BSD 3-Clause License.
The full license agreement can be found in the LICENSE file.

This software is distributed without any warranty.
*/

#pragma once
#ifndef WOODYCXX_NET_SOCKETEXCPETION_H_
#define WOODYCXX_NET_SOCKETEXCPETION_H_

#include <io/IOException.h>

using namespace std;
using namespace woodycxx::io;

namespace woodycxx { namespace net {

class SocketException : public IOException
{
public:
	SocketException(const string& errMsg) : IOException(errMsg)
    {
        exception_name = "SocketException: ";
    }

	SocketException(int errcode) : IOException(errcode)
	{
		exception_name = "SocketException: ";
	}

    virtual ~SocketException() {}

    };

}} //end of namespace woodycxx::io

#endif