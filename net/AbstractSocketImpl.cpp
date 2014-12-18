#include "AbstractSocketImpl.h"
#include <iostream>

namespace woodycxx { namespace net {

void AbstractSocketImpl::connect(string host, int port)
{

}

void AbstractSocketImpl::connect(InetSocketAddress& address, int port)
{
    std::cout << "AbstractSocketImpl::connect" << std::endl;
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
