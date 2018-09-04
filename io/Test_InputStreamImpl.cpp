/*
  Copyright (c) 2014-2015 by NASa Qian <nasacj@nasacj.net>
  This file is part of the woodycxx library.

  This software is distributed under BSD 3-Clause License.
  The full license agreement can be found in the LICENSE file.

  This software is distributed without any warranty.
*/

#include <iostream>
#include "InputStreamImpl.h"

using namespace woodycxx::io;

#if 1

int main() {
  ByteBuffer buf(10);
  InputStreamImpl inputstreamimpl;
  InputStream &inputstream = inputstreamimpl;
  inputstream.read(buf);

  for (int i = 0; i < buf.getSize(); i++) {
    std::cout << static_cast<int>(buf[i]) << " ";
  }
  std::cout << std::endl;

  InputStreamImpl impl;
  impl.InputStream::read(buf);
  //impl.read(buf);

  char *bytes = new char[10];
  inputstream.read(bytes, 10);

  std::cout << "bytes new[10] reading..." << std::endl;
  for (int i = 0; i < buf.getSize(); i++) {
    std::cout << static_cast<int>(buf[i]) << " ";
  }
  std::cout << std::endl;

  impl.hello();

  return 0;
}

#endif
