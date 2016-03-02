/*
  Copyright (c) 2014-2015 by NASa Qian <nasacj@nasacj.net>
  This file is part of the woodycxx library.

  This software is distributed under BSD 3-Clause License.
  The full license agreement can be found in the LICENSE file.

  This software is distributed without any warranty.
*/

#ifndef Woodycxx_Net_Sockets_SocketsOpt_H_
#define Woodycxx_Net_Sockets_SocketsOpt_H_

#include<stdint.h>
#ifdef WIN32
#include <WinSock2.h>
#include <ws2ipdef.h>
#else
#include <arpa/inet.h>
#include <unistd.h>
#endif

namespace woodycxx { namespace net { namespace sockets {

int createBlockingSocketFd(bool isIpv6=false);
int connect(int sockfd, uint16_t family, const struct sockaddr * addr);
int bind(int sockfd, const struct sockaddr_in& addr);
int listen(int sockfd);
int read(int sockfd, void *buf, size_t count);
#ifndef WIN32
int readv(int sockfd, const struct iovec *iov, int iovcnt);
#endif
int write(int sockfd, const void *buf, size_t count);
int shutdownWrite();
int close(int sockfd);
void toIpPort(char* buf, size_t size, const struct sockaddr *sa);
void toIp(char* buf, size_t size, const struct sockaddr *sa);
void toIp(char* buf, size_t size, const void *addr, uint16_t family);
void fromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr);
void fromIpPort(const char* ip, uint16_t port, struct sockaddr_in6* addr);
void fromAddrPort(const struct in_addr& address, uint16_t port, struct sockaddr_in* addr);
void fromAddrPort(const struct in6_addr& address, uint16_t port, struct sockaddr_in6* addr);

const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr);
const struct sockaddr* sockaddr_cast(const struct sockaddr_in6* addr);

int getSocketError(int sockfd);

}}}//end of namespace woodycxx::net::sockets

#endif
