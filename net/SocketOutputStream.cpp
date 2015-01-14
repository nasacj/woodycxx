#include "SocketOutputStream.h"
#include "SocketsOpt.h"

namespace woodycxx { namespace net {

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
    if ( 0 == b )
        return woodycxx::error::NullPointerError;
    else if ( (off < 0) || (off > b_size) || (len < 0) ||
        ( (off + len) > b_size) || ((off + len) < 0))
    {
        return woodycxx::error::BufferIndexOutOfBounds;
    }
    else if (len == 0)
    {
        return 0;
    }

    if (NULL == socket_impl)
        return woodycxx::error::NullPointerError;

    if (socket_impl->isConnectionReset()) 
        return woodycxx::error::ConnectionReset;

    FileDescriptor& fd = socket_impl->getFileDescriptor();
    return socketWrite0(fd, b, b_size, off, len);
}

int SocketOutputStream::socketWrite0(FileDescriptor& fd, const void* b, int b_size, int off, int len)
{
    if ( 0 == b )
        return woodycxx::error::NullPointerError;
    else if ( (off < 0) || (off > b_size) || (len < 0) ||
        ( (off + len) > b_size) || ((off + len) < 0))
    {
        return woodycxx::error::BufferIndexOutOfBounds;
    }
    else if (len == 0)
    {
        return 0;
    }

    int socketfd = static_cast<int>(fd.getHandler());
    const char* byte_buf = static_cast<const char*>(b);
    int write_count = sockets::write(socketfd, &byte_buf[off], len);
    if ( write_count != len )
    {
        //TODO Handle error?
        return write_count;
    }
    return write_count;
}

void SocketOutputStream::flush()
{
}

void SocketOutputStream::close()
{
    if (socket_impl != NULL)
    {
        if (!socket_impl->isClosed())
        {
            socket_impl->close();
        }
    }
}

}}//end of namespace woodycxx::net