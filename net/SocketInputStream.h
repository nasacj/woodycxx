/*
  Copyright (c) 2014-2015 by NASa Qian <nasacj@nasacj.net>
  This file is part of the woodycxx library.

  This software is distributed under BSD 3-Clause License.
  The full license agreement can be found in the LICENSE file.

  This software is distributed without any warranty.
*/

#ifndef SOCKET_INPUT_STREAM_H_
#define SOCKET_INPUT_STREAM_H_

#include <io/InputStream.h>
#include <io/FileDescriptor.h>
#include "AbstractSocketImpl.h"
//#include <boost/shared_ptr.hpp>
#include <memory>

using namespace std;
using namespace woodycxx::io;

namespace woodycxx { namespace net {

class AbstractSocketImpl;
typedef shared_ptr<AbstractSocketImpl> AbstractSocketImplPtr;

class SocketInputStream : public InputStream
{
private:
    AbstractSocketImplPtr socket_impl;
    bool eof;
    //bool closing;
    //FileDescriptor fd;

public:
    //SocketInputStream(){}
    SocketInputStream(const AbstractSocketImplPtr& impl) : socket_impl(impl), eof(false)
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
