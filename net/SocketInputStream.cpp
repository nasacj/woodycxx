/*
  Copyright (c) 2014-2015 by NASa Qian <nasacj@nasacj.net>
  This file is part of the woodycxx library.

  This software is distributed under BSD 3-Clause License.
  The full license agreement can be found in the LICENSE file.

  This software is distributed without any warranty.
*/

#include "SocketInputStream.h"
//#include <smart_ptr/scoped_array.h>
#include <boost/scoped_array.hpp>
#include <sys/types.h>
#include "SocketsOpt.h"

namespace woodycxx { namespace net {

int SocketInputStream::read()
{
    if (eof) {
        return -1;
    }
    //typedef woodycxx::smart_prt::scoped_array<char> Byte_Array;
    typedef boost::scoped_array<char> Byte_Array;
    char* temp = Byte_Array(new char[1]).get();
    int n = read(temp, 1);
    if (n <= 0) {
        return -1;
    }
    return temp[0] & 0xff;
}

int SocketInputStream::read(char* b, int len)
{
    return read(b, len, 0, len);
}

int SocketInputStream::read(char* b, int buf_size, int off, int len)
{
    return read(b, buf_size, off, len, socket_impl->getTimeout());
}

int SocketInputStream::read(char* b, int byte_size, int off, int length, int timeout)
{
    int n = 0;

    // EOF already encountered
    if (eof) {
        return -1;
    }

    if (NULL == socket_impl)
        return woodycxx::error::NullPointerError;

    if (socket_impl->isConnectionReset()) 
        return woodycxx::error::ConnectionReset;

    // bounds check
    if (length <= 0 || off < 0 || off + length > byte_size)
    {
        if (length == 0)
        {
            return 0;
        }
        return woodycxx::error::BufferIndexOutOfBounds;
    }

    FileDescriptor fd = socket_impl->getFileDescriptor();
    n = socketRead0(fd, byte_size, b, off, length, timeout);
    if (n > 0) {
        return n;
    }

    eof = true;
    return -1;

}

void SocketInputStream::close()
{
    if (socket_impl != NULL)
    {
        if (!socket_impl->isClosed())
        {
            socket_impl->close();
        }
    }
}

int SocketInputStream::socketRead0(FileDescriptor& fd, int buf_size, char* b, int off, int len, int timeout)
{
    if (len <= 0 || off < 0 || off + len > buf_size)
    {
        if (len == 0)
        {
            return 0;
        }
        return woodycxx::error::BufferIndexOutOfBounds;
    }

    int socketfd = static_cast<int>(fd.getHandler());
    int n = 0;
    n = sockets::read(socketfd, &b[off], len);

    //TODO Timeout handling

    return n;
}


}}//end of namespace

