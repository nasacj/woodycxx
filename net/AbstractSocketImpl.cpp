#include "AbstractSocketImpl.h"
#include "SocketsOpt.h"

namespace woodycxx { namespace net {


int AbstractSocketImpl::connect(string host, int port)
{
    InetAddress addr(host, port);
    return connect(addr);
}

int AbstractSocketImpl::connect(InetAddress& address)
{
    int sockfd = sockets::createBlockingSocketFd();
#ifdef WIN32
    if ( sockfd == INVALID_SOCKET )
#else
    if ( sockfd < 0 )
#endif
    {
        return woodycxx::error::SocketError;
    }
    
    int ret = sockets::connect(sockfd, address.getSockAddrInet());
    if ( ret < 0 )
        return woodycxx::error::ConnectionError;

    this->fileHandler.set(sockfd);
    this->connected = true; 
    return 0;    
}

int AbstractSocketImpl::connect()
{
    return this->connect(this->address);
}

void AbstractSocketImpl::bind(InetAddress& host)
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
