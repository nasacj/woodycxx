/*
 * SocketOutputStream.h
 *
 *  Created on: 2014-12-18
 *      Author: qianchj
 */

#ifndef SOCKET_OUTPUT_STREAM_H_
#define SOCKET_OUTPUT_STREAM_H_

#include <io/OutputStream.h>
#include <io/FileDescriptor.h>
#include "AbstractSocketImpl.h"

using namespace woodycxx::io;

namespace woodycxx { namespace net {

class AbstractSocketImpl;

class SocketOutputStream : public OutputStream
{
private:
    AbstractSocketImpl* socket_impl;

public:
    SocketOutputStream(AbstractSocketImpl* impl) : socket_impl(impl) {}
    ~SocketOutputStream(){}

    int write(char b);

    int write(const void* b, int len);

    int write(const void* b, int b_size, int off, int len);

    void flush();

    void close();

private:
    int socketWrite0(FileDescriptor& fd, const void* b, int b_size, int off, int len);
};



}}//end of namespace woodycxx::net
#endif