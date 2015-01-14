/*
 * SocketsOpt.h
 *
 *  Created on: 2015-1-13
 *      Author: qianchj
 */

#ifndef Woodycxx_Net_Sockets_SocketsOpt_H_
#define Woodycxx_Net_Sockets_SocketsOpt_H_

#include<stdint.h>
#ifdef WIN32
#include <WinSock2.h>
#else
#include <arpa/inet.h>
#endif

namespace woodycxx { namespace net { namespace sockets {

int createBlockingSocketFd();
int connect(int sockfd, const struct sockaddr_in& addr);
int read(int sockfd, void *buf, size_t count);
#ifndef WIN32
int readv(int sockfd, const struct iovec *iov, int iovcnt);
#endif
int write(int sockfd, const void *buf, size_t count);
int close(int sockfd);
void toIpPort(char* buf, size_t size, const struct sockaddr_in& addr);
void toIp(char* buf, size_t size, const struct sockaddr_in& addr);
void fromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr);

}}}//end of namespace woodycxx::net::sockets

#endif