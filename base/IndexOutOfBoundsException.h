/*
Copyright (c) 2014-2015 by NASa Qian <nasacj@nasacj.net>
This file is part of the woodycxx library.

This software is distributed under BSD 3-Clause License.
The full license agreement can be found in the LICENSE file.

This software is distributed without any warranty.
*/

#pragma once
#ifndef WOODYCXX_BASE_INDEXOUTOFBOUNDSEXCEPTION_H_
#define WOODYCXX_BASE_INDEXOUTOFBOUNDSEXCEPTION_H_

#include "RuntimeException.h"

namespace woodycxx {
namespace base {

class IndexOutOfBoundsException : public RuntimeException {
 public:
  IndexOutOfBoundsException(const string &errMsg) : RuntimeException(errMsg) {
    exception_name = "IndexOutOfBoundsException: ";
  }

  virtual ~IndexOutOfBoundsException() {}

};

}
} //end of namespace woodycxx::base

#endif