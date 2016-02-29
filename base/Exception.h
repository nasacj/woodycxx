/*
Copyright (c) 2014-2015 by NASa Qian <nasacj@nasacj.net>
This file is part of the woodycxx library.

This software is distributed under BSD 3-Clause License.
The full license agreement can be found in the LICENSE file.

This software is distributed without any warranty.
*/

#pragma once
#ifndef WOODYCXX_BASE_EXCETPION_H_
#define WOODYCXX_BASE_EXCETPION_H_

#include <exception>
#include <string>

using namespace std;

namespace woodycxx { namespace base {

class Exception : public exception
{
protected:
	string exception_name;
	string what_msg;
	string detailed_errmsg;
public:
	Exception(const string& errMsg) : what_msg(errMsg)
	{
		exception_name = "UnkonwnException: ";
	}

	virtual ~Exception() {}

	virtual char const* what() 
	{
		detailed_errmsg = exception_name + what_msg;
		return detailed_errmsg.c_str();
	}
};

}} //end of namespace woodycxx::io

#endif
