/*
Copyright (c) 2014-2015 by NASa Qian <nasacj@nasacj.net>
This file is part of the woodycxx library.

This software is distributed under BSD 3-Clause License.
The full license agreement can be found in the LICENSE file.

This software is distributed without any warranty.
*/

#pragma once
#ifndef WOODYCXX_IO_IOEXCETPION_H_
#define WOODYCXX_IO_IOEXCETPION_H_

#include <exception>
#include <string>
#include <base/Exception.h>

using namespace std;
using namespace woodycxx::base;

namespace woodycxx { namespace io {

class IOException : public Exception
{
public:
	IOException(const string& errMsg) : Exception(errMsg)
	{
		exception_name = "IOException: ";
	}

	virtual ~IOException() {}

};

}} //end of namespace woodycxx::io

#endif
