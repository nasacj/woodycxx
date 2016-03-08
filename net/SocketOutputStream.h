/*
  Copyright (c) 2014-2015 by NASa Qian <nasacj@nasacj.net>
  This file is part of the woodycxx library.

  This software is distributed under BSD 3-Clause License.
  The full license agreement can be found in the LICENSE file.

  This software is distributed without any warranty.
*/

#ifndef SOCKET_OUTPUT_STREAM_H_
#define SOCKET_OUTPUT_STREAM_H_

#include <io/OutputStream.h>
#include <io/FileDescriptor.h>
#include "AbstractSocketImpl.h"
#include "SocketException.h"
#include <base/IndexOutOfBoundsException.h>
//#include <boost/shared_ptr.hpp>
#include <memory>

using namespace std;
using namespace woodycxx::io;

namespace woodycxx { namespace net {

class AbstractSocketImpl;
typedef shared_ptr<AbstractSocketImpl> AbstractSocketImplPtr;

class SocketOutputStream : public OutputStream
{
private:
    AbstractSocketImplPtr socket_impl;
    FileDescriptor socketFD;

public:
    SocketOutputStream(const AbstractSocketImplPtr& impl) : socket_impl(impl)
    {
        init();
    }

    ~SocketOutputStream(){}

    int write(char b);

    int write(const void* b, int len);

    int write(const void* b, int b_size, int off, int len);

    void flush();

    void close();

private:
    void init();
    int socketWrite0(FileDescriptor& fd, const void* b, int b_size, int off, int len);
};



}}//end of namespace woodycxx::net
#endif
