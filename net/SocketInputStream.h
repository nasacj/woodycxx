/*
 * SocketInputStream.h
 *
 *  Created on: 2014-12-17
 *      Author: qianchj
 */

#ifndef SOCKET_INPUT_STREAM_H_
#define SOCKET_INPUT_STREAM_H_

#include <io/InputStream.h>
#include <io/FileDescriptor.h>
#include "AbstractSocketImpl.h"

using namespace woodycxx::io;

namespace woodycxx { namespace net {

class AbstractSocketImpl;

class SocketInputStream : public InputStream
{
private:
    AbstractSocketImpl* socket_impl;
    bool eof;
    //bool closing;
    //FileDescriptor fd;

public:
    //SocketInputStream(){}
    SocketInputStream(AbstractSocketImpl* impl) : socket_impl(impl)
    {
        //fd = impl->getFileDescriptor();
    }
    ~SocketInputStream(){}

    int read();

    int read(char* b, int len);

    int read(char* b, int buf_size, int off, int len);

    void close();

protected:
    int read(char* b, int byte_size, int off, int length, int timeout);

private:
    int socketRead0(FileDescriptor& fd, int buf_size, char* b, int off, int len, int timeout);

};

}}//end of namespace woodycxx::net

#endif