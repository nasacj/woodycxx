/*
  Copyright (c) 2014-2015 by NASa Qian <nasacj@nasacj.net>
  This file is part of the woodycxx library.

  This software is distributed under BSD 3-Clause License.
  The full license agreement can be found in the LICENSE file.

  This software is distributed without any warranty.
*/

#ifndef FILE_DESCRIPTOR_H_
#define FILE_DESCRIPTOR_H_

#include <base/Types.h>

namespace woodycxx {
namespace io {

typedef int Handle;

class FileDescriptor {
 private:
  Handle handler;

 public:
  FileDescriptor() {}
  ~FileDescriptor() {}
  explicit FileDescriptor(Handle handle_n) : handler(handle_n) {}
  Handle getHandler() { return this->handler; }
  void set(Handle handler_ot) { this->handler = handler_ot; }
};

}
}

#endif
