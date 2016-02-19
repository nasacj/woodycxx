/*
  Copyright (c) 2014-2015 by NASa Qian <nasacj@nasacj.net>
  This file is part of the woodycxx library.

  This software is distributed under BSD 3-Clause License.
  The full license agreement can be found in the LICENSE file.

  This software is distributed without any warranty.
*/

#include "SocketsOpt.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#ifdef WIN32
#include <WinSock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib,"ws2_32.lib")
#else
#include <unistd.h>
#include <sys/socket.h>
#endif
#ifdef __APPLE__
#include <sys/uio.h>
#endif

using namespace woodycxx::net;

namespace woodycxx { namespace net { namespace sockets {

typedef struct sockaddr SA;

#ifdef WIN32
class InitialWindowsSocketAPI
{
public:
    InitialWindowsSocketAPI()
    {
        WSADATA wsa;
        if(WSAStartup(MAKEWORD(2,2),&wsa) !=0 )
        {
            //TODO: handle this error;
        }
    }

    ~InitialWindowsSocketAPI()
    {
        WSACleanup();
    }
};
InitialWindowsSocketAPI initialwindowssocketapi;
#endif

int createBlockingSocketFd()
{
    int sockfd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    return sockfd;
}

int connect(int sockfd, const struct sockaddr * addr)
{
    int ret = ::connect( sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)) );
    return ret;
}

int bind(int sockfd, const struct sockaddr_in& addr)
{
    int ret = ::bind(sockfd,(struct sockaddr *)(&addr), static_cast<socklen_t>(sizeof addr));
    return ret;
}

int listen(int sockfd)
{
    int ret = ::listen(sockfd, SOMAXCONN);
    return ret;
}

int read(int sockfd, void *buf, size_t count)
{
#ifdef WIN32
    return ::recv(sockfd, (char*)buf, count, 0);
#else
    return ::read(sockfd, buf, count);
#endif
}

#ifndef WIN32
int readv(int sockfd, const struct iovec *iov, int iovcnt)
{
    return ::readv(sockfd, iov, iovcnt);
}
#endif

int write(int sockfd, const void *buf, size_t count)
{
#ifdef WIN32
    return ::send(sockfd, (const char*)buf, count, 0);
#else
    return ::write(sockfd, buf, count);
#endif
}

int shutdownWrite(int sockfd)
{
#ifdef WIN32
    return ::shutdown(sockfd, SD_SEND);
#else
    return ::shutdown(sockfd, SHUT_WR);
#endif
}

int close(int sockfd)
{
#ifdef WIN32
    return ::closesocket(sockfd);
#else
    return ::close(sockfd);
#endif
}

void toIpPort(char* buf, size_t size, const struct sockaddr *sa)
{
	assert(size >= INET6_ADDRSTRLEN || size >= INET_ADDRSTRLEN);

	switch (sa->sa_family) {
		case AF_INET: {
			struct sockaddr_in	*addr = (struct sockaddr_in *) sa;

#ifdef WIN32
			if (inet_ntop(AF_INET, (PVOID)(&(addr->sin_addr)), buf, size) == NULL)
#else
			if (inet_ntop(AF_INET, &addr->sin_addr, buf, static_cast<socklen_t>(size)) == NULL)
#endif
			{
				buf = NULL;
				return;
			}
			size_t end = strlen(buf);
			assert(size > end);
			uint16_t port = 0;
			if ((port = ntohs(addr->sin_port)) != 0) {
#ifdef WIN32
				sprintf_s(buf + end, size - end, ":%u", port);
#else
				snprintf(buf + end, size - end, ":%u", port);
#endif
			}
			return;
		}

		case AF_INET6: {
			struct sockaddr_in6	*addr = (struct sockaddr_in6 *) sa;
			buf[0] = '[';
#ifdef WIN32
			if (inet_ntop(AF_INET6, (PVOID)(&(addr->sin6_addr)), buf + 1, size) == NULL)
#else
			if (inet_ntop(AF_INET6, &addr->sin6_addr, buf + 1, static_cast<socklen_t>(size)) == NULL)
#endif
			{
				buf = NULL;
				return;
			}
			size_t end = strlen(buf);
			assert(size > end);
			uint16_t port = ntohs(addr->sin6_port);
#ifdef WIN32
			sprintf_s(buf + end, size - end, "]:%u", port);
#else
			snprintf(buf + end, size - end, "]:%u", port);
#endif
			return;
		}
	}
}

void toIp(char* buf, size_t size, const struct sockaddr *sa)
{
	assert(size >= INET6_ADDRSTRLEN || size >= INET_ADDRSTRLEN);

	switch (sa->sa_family) {
	case AF_INET: {
		struct sockaddr_in	*addr = (struct sockaddr_in *) sa;

#ifdef WIN32
		if (inet_ntop(AF_INET, (PVOID)(&(addr->sin_addr)), buf, size) == NULL)
#else
		if (inet_ntop(AF_INET, &addr->sin_addr, buf, static_cast<socklen_t>(size)) == NULL)
#endif
		{
			buf = NULL;
		}
		return;
	}// end of AF_INET:

	case AF_INET6: {
		struct sockaddr_in6	*addr = (struct sockaddr_in6 *) sa;
		buf[0] = '[';
#ifdef WIN32
		if (inet_ntop(AF_INET6, (PVOID)(&(addr->sin6_addr)), buf + 1, size) == NULL)
#else
		if (inet_ntop(AF_INET6, &addr->sin6_addr, buf + 1, static_cast<socklen_t>(size)) == NULL)
#endif
		{
			buf = NULL;
			return;
		}
		int end = strlen(buf);
		buf[end] = ']';
		buf[end + 1] = 0;
		return;
	} // end of  AF_INET6:
		

	} // end of switch
}

void fromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr)
{
    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);
    if ( inet_pton(AF_INET, ip, &addr->sin_addr) <= 0)
    {
        //TODO: Handle invalid IP address format error
		return;
    }
}

void fromIpPort(const char* ip, uint16_t port, struct sockaddr_in6* addr)
{
	addr->sin6_family = AF_INET6;
	addr->sin6_port = htons(port);
	if (inet_pton(AF_INET6, ip, &addr->sin6_addr) <= 0)
	{
		//TODO: Handle invalid IP address format error
		return;
	}
}

const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr)
{
	return (struct sockaddr*)(addr);
}

const struct sockaddr* sockaddr_cast(const struct sockaddr_in6* addr)
{
	return (struct sockaddr*)(addr);
}


}}}//end of namespace
