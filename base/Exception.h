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
#include <functional>
#ifdef WIN32
//#include <base/StackWalker.h>
#include <WinSock2.h>
#include <ws2ipdef.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <string.h> //strerror
#endif // WIN32

using namespace std;

namespace woodycxx { namespace base {

class Exception : public exception
{
protected:
	string exception_name;
	string what_msg;
	string exception_msg;
public:
	Exception(const string& errMsg) : what_msg(errMsg)
	{
		exception_name = "UnkonwnException: ";
		//StackWalker sw; sw.ShowCallstack();
	}

	Exception(int errcode)
	{
		exception_name = "UnkonwnException: ";
#ifdef WIN32
		WCHAR* msg = gai_strerror(errcode);
		DWORD num = WideCharToMultiByte(CP_ACP, 0, msg, -1, NULL, 0, NULL, 0);
		char *cword = new char[num];
		ZeroMemory(cword, num);
		WideCharToMultiByte(CP_ACP, 0, msg, -1, cword, num, NULL, 0);
		what_msg = cword;
		delete[](cword);
#else
		what_msg = gai_strerror(errcode);
#endif
	}

	virtual ~Exception() {}

	virtual char const* what() noexcept 
	{
		exception_msg = exception_name + what_msg;
		return exception_msg.c_str();
	}

	static inline std::string GetLastErrorAsString()
	{
#ifdef WIN32
		
		//Get the error message, if any.
		DWORD errorMessageID = ::GetLastError();
		if (errorMessageID == 0)
			return std::string(); //No error message has been recorded

		LPSTR messageBuffer = nullptr;
		size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

		std::string message(messageBuffer, size);

		//Free the buffer.
		LocalFree(messageBuffer);

		return message;		

#else
		char * mesg = strerror(errno);
		return string(mesg);
#endif
	}

};

typedef std::function<void(void)> FinalCallBack;

class Finally
{
private:
	FinalCallBack finalCallBack;
public:
	Finally(const FinalCallBack& callback) : finalCallBack(callback) {}
	~Finally() { finalCallBack(); }
};

}} //end of namespace woodycxx::io

#endif
