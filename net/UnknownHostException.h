/*
Copyright (c) 2014-2015 by NASa Qian <nasacj@nasacj.net>
This file is part of the woodycxx library.

This software is distributed under BSD 3-Clause License.
The full license agreement can be found in the LICENSE file.

This software is distributed without any warranty.
*/

#pragma once
#ifndef WOODYCXX_NET_UNKNOWNHOSTEXCEPTION_H_
#define WOODYCXX_NET_UNKNOWNHOSTEXCEPTION_H_

#include <io/IOException.h>

using namespace std;
using namespace woodycxx::io;

namespace woodycxx { namespace net {

class UnknownHostException : public IOException
{
public:
	UnknownHostException(const string& errMsg) : IOException(errMsg)
	{
		exception_name = "UnknownHostException: ";
	}

	virtual ~UnknownHostException() {}

};

}} //end of namespace woodycxx::io

#endif

