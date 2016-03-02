/*
  Copyright (c) 2014-2015 by NASa Qian <nasacj@nasacj.net>
  This file is part of the woodycxx library.

  This software is distributed under BSD 3-Clause License.
  The full license agreement can be found in the LICENSE file.

  This software is distributed without any warranty.
*/

#include "AbstractSocketImpl.h"
#include "SocketsOpt.h"
#include <sysipc/sysipc_event_log.h>
#ifdef WIN32
#include <WinSock2.h>
#include <ws2tcpip.h>
#else
#include <strings.h>   //bzero
#include <sys/socket.h>
#include <netdb.h>
#endif

#define DEBUG_LOG(message) DEBUG_INFO("AbstractSocketImpl", message)

namespace woodycxx { namespace net {


int AbstractSocketImpl::connect(const string& host, int port)
{
    if (connected)
        return 0;
	InetSocketAddress addr(host, port);
    return connect(addr);
}

int AbstractSocketImpl::connect(const InetSocketAddress& addr)
{
    if (connected)
    {
        return 0;
    }
    this->address = addr;
    int sockfd = sockets::createBlockingSocketFd(address.isIPv6());
#ifdef WIN32
    if ( sockfd == INVALID_SOCKET )
#else
    if ( sockfd < 0 )
#endif
    {
        return woodycxx::error::SocketError;
    }

    int ret = sockets::connect(sockfd, address.getFamily() , address.getSockAddrP());
    if (ret < 0)
    {
		string errMsg;
#ifdef _WINDOWS_
		WCHAR* msg = gai_strerror(ret);
		DWORD num = WideCharToMultiByte(CP_ACP, 0, msg, -1, NULL, 0, NULL, 0);
		char *cword = new char[num];
		bzero(cword, num);
		WideCharToMultiByte(CP_ACP, 0, msg, -1, cword, num, NULL, 0);
		errMsg = string(cword);
		delete[](cword);
		DEBUG_LOG("Connect Failed: " << errMsg);
#else
		ret = sockets::getSocketError(sockfd);
		DEBUG_LOG("Connect Failed: " << strerror(ret));
#endif

		return woodycxx::error::ConnectionError;
	}

    this->fileHandler.set(sockfd);
    this->connected = true;
    return 0;
}

int AbstractSocketImpl::connect()
{
    return this->connect(this->address);
}

void AbstractSocketImpl::bind(const InetSocketAddress& host)
{
}

void AbstractSocketImpl::listen(int backlog)
{
}


void AbstractSocketImpl::accept(const AbstractSocket& s)
{
}


InputStreamPtr AbstractSocketImpl::getInputStream()
{
    //return make_shared<SocketInputStream>(shared_from_this());
    if ( wkInputStreamPtr.expired() )
    {
        InputStreamPtr inputstrPtr = make_shared<SocketInputStream>(shared_from_this());
        wkInputStreamPtr = inputstrPtr;
        return inputstrPtr;
    }
    return wkInputStreamPtr.lock();
}


OutputStreamPtr AbstractSocketImpl::getOutputStream()
{
    //return make_shared<SocketOutputStream>(shared_from_this());
    if ( wkOutputStreamPtr.expired() )
    {
        OutputStreamPtr outputstrPtr = make_shared<SocketOutputStream>(shared_from_this());
        wkOutputStreamPtr = outputstrPtr;
        return outputstrPtr;
    }
    return wkOutputStreamPtr.lock();
}


int AbstractSocketImpl::available()
{
    return 0;
}


void AbstractSocketImpl::close()
{
    sockets::close(fileHandler.getHandler());
}

string AbstractSocketImpl::getIpString()
{
    return this->address.getHostAddress();
}

uint16_t AbstractSocketImpl::getPortString()
{
    return this->address.getPort();
}

string AbstractSocketImpl::getIpPortString()
{
    return this->address.toString();
}

}}
