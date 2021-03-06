/*
  Copyright (c) 2014-2015 by NASa Qian <nasacj@nasacj.net>
  This file is part of the woodycxx library.

  This software is distributed under BSD 3-Clause License.
  The full license agreement can be found in the LICENSE file.

  This software is distributed without any warranty.
*/

#include "SocketInputStream.h"
//#include <smart_ptr/scoped_array.h>
#include "SocketsOpt.h"

#ifndef TEMP_FAILURE_RETRY
#define TEMP_FAILURE_RETRY
#endif

namespace woodycxx {
namespace net {

void SocketInputStream::init() {
  socketFD = socket_impl->getFileDescriptor();
}

int SocketInputStream::read() {
  if (eof) {
    return -1;
  }
  //typedef woodycxx::smart_prt::scoped_array<char> Byte_Array;
  typedef boost::scoped_array<char> Byte_Array;
  char *temp = Byte_Array(new char[1]).get();
  int n = read(temp, 1);
  if (n <= 0) {
    return -1;
  }
  return temp[0] & 0xff;
}

int SocketInputStream::read(char *b, int len) {
  return read(b, len, 0, len);
}

int SocketInputStream::read(char *b, int buf_size, int off, int len) {
  return read(b, buf_size, off, len, socket_impl->getTimeout());
}

int SocketInputStream::read(char *b, int byte_size, int off, int length, int timeout) {
  int n = 0;

  // EOF already encountered
  if (eof) {
    return -1;
  }

  if (socket_impl->isConnectionReset())
    throw SocketException("Connection Reset");

  // bounds check
  if (length <= 0 || off < 0 || off + length > byte_size) {
    if (length == 0) {
      return 0;
    }
    throw IndexOutOfBoundsException("SocketInputStream::read(): buffer bounds check failed!");
  }

  n = socketRead0(socketFD, byte_size, b, off, length, timeout);
  if (n > 0) {
    return n;
  }

  eof = true;

  return n;

}

void SocketInputStream::close() {
  weak_ptr<AbstractSocketImpl> wkAbSocImp = socket_impl;
  if (!wkAbSocImp.expired()) {
    if (!socket_impl->isCloseRead() && !socket_impl->isClosed()) {
      socket_impl->closeRead();
    }
  }
}

int SocketInputStream::socketRead0(FileDescriptor &fd, int buf_size, char *b, int off, int len, int timeout) {
  if (len <= 0 || off < 0 || off + len > buf_size) {
    if (len == 0) {
      return 0;
    }
    return woodycxx::error::BufferIndexOutOfBounds;
  }

  int socketfd = static_cast<int>(fd.getHandler());
  int n = 0;
  n = TEMP_FAILURE_RETRY(sockets::read(socketfd, &b[off], len));

  if (n < 0) {
    if (errno == EINTR) {
      return 0;
    }
    socket_impl->setConnectionReset();
    throw SocketException("read: " + Exception::GetLastErrorAsString());
  }
  //TODO Timeout handling

  return n;
}

}
}//end of namespace

