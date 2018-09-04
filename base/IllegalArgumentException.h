/*
Copyright (c) 2014-2015 by NASa Qian <nasacj@nasacj.net>
This file is part of the woodycxx library.

This software is distributed under BSD 3-Clause License.
The full license agreement can be found in the LICENSE file.

This software is distributed without any warranty.
*/

#pragma once
#ifndef WOODYCXX_BASE_ILLEGALARGUMENTEXCEPTION_H_
#define WOODYCXX_BASE_ILLEGALARGUMENTEXCEPTION_H_

#include <exception>
#include <string>
#include <base/Exception.h>

using namespace std;

namespace woodycxx {
namespace base {

class IllegalArgumentException : public Exception {
 public:
  IllegalArgumentException(const string &errMsg) : Exception(errMsg) {
    exception_name = "IllegalArgumentException: ";
  }

  virtual ~IllegalArgumentException() {}

};

}
} //end of namespace woodycxx::base

#endif

