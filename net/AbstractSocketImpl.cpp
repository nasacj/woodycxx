#include "AbstractSocketImpl.h"
#include <iostream>
#include <sys/types.h>
#ifdef WIN32
#include <WinSock.h>
#pragma comment(lib,"ws2_32.lib")
#else
#include <sys/socket.h>
#endif

namespace woodycxx { namespace net {

int AbstractSocketImpl::connect(string host, int port)
{
#ifdef WIN32
    WSADATA wsa;
    if(WSAStartup(MAKEWORD(2,2),&wsa) !=0 )
        return woodycxx::error::SocketError;
    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if ( sockfd == INVALID_SOCKET )
#else
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if ( sockfd < 0 )
#endif
        return woodycxx::error::SocketError;

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
#ifdef WIN32
    servaddr.sin_addr.S_un.S_addr = inet_addr(host.c_str());
    if ( INADDR_NONE == servaddr.sin_addr.S_un.S_addr)
#else
    if (inet_pton(AF_INET, host.c_str(), &servaddr.sin_addr) < 0 )
#endif
        return woodycxx::error::InvalidIpAddress;
    if (::connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
        return woodycxx::error::ConnectionError;

    this->fileHandler.set(sockfd);
    this->connected = true; 
    return 0;    
}

int AbstractSocketImpl::connect(InetSocketAddress& address, int port)
{
    std::cout << "AbstractSocketImpl::connect" << std::endl;
    return connect(address.getHostName(), port);
}

void AbstractSocketImpl::bind(InetSocketAddress& host, int port)
{
}

void AbstractSocketImpl::listen(int backlog)
{
}


void AbstractSocketImpl::accept(AbstractSocket& s)
{
}


InputStream& AbstractSocketImpl::getInputStream()
{
    if ( NULL == inputStreamPtr.get() )
    {
        inputStreamPtr.reset(new SocketInputStream(this));
    }
    return *inputStreamPtr;
}


OutputStream& AbstractSocketImpl::getOutputStream()
{
    if ( NULL == outputStreamPtr.get() )
    {
        outputStreamPtr.reset(new SocketOutputStream(this));
    }
    return *outputStreamPtr;
}


int AbstractSocketImpl::available()
{
    return 0;
}


void AbstractSocketImpl::close()
{
}


}}
