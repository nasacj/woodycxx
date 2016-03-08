/*
  Copyright (c) 2014-2015 by NASa Qian <nasacj@nasacj.net>
  This file is part of the woodycxx library.

  This software is distributed under BSD 3-Clause License.
  The full license agreement can be found in the LICENSE file.

  This software is distributed without any warranty.
*/

#include "SocketOutputStream.h"
#include "SocketsOpt.h"

namespace woodycxx { namespace net {

void SocketOutputStream::init()
{
    socketFD = socket_impl->getFileDescriptor();
}

int SocketOutputStream::write(char b)
{
    return write(&b, 1, 0, 1);
}

int SocketOutputStream::write(const void* b, int len)
{
    return write(b, len, 0, len);
}

int SocketOutputStream::write(const void* b, int b_size, int off, int len)
{

    if ( (off < 0) || (off > b_size) || (len < 0) ||
        ( (off + len) > b_size) || ((off + len) < 0))
    {
        throw IndexOutOfBoundsException("SocketOutputStream::write: buffer size out-of-bounds");
    }
    else if (len == 0)
    {
        return 0;
    }

    if (socket_impl->isConnectionReset()) 
        throw SocketException("Connection reset");

    return socketWrite0(socketFD, b, b_size, off, len);
}

int SocketOutputStream::socketWrite0(FileDescriptor& fd, const void* b, int b_size, int off, int len)
{
    int socketfd = static_cast<int>(fd.getHandler());
    const char* byte_buf = static_cast<const char*>(b);
    int write_count = sockets::write(socketfd, &byte_buf[off], len);
    if ( write_count < 0 )
    {
		socket_impl->setConnectionReset();
		throw SocketException("write: " + Exception::GetLastErrorAsString());
    }
    return write_count;
}

void SocketOutputStream::flush()
{
}

void SocketOutputStream::close()
{
    weak_ptr<AbstractSocketImpl> wkAbSocImp = socket_impl;
    if ( !wkAbSocImp.expired() )
    {
        if (!socket_impl->isClosed())
        {
            socket_impl->close();
        }
    }
}

}}//end of namespace woodycxx::net
