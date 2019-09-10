/*
  Copyright (c) 2014-2015 by NASa Qian <nasacj@nasacj.net>
  This file is part of the woodycxx library.

  This software is distributed under BSD 3-Clause License.
  The full license agreement can be found in the LICENSE file.

  This software is distributed without any warranty.
*/

#ifndef WOODYCXX_BASIC_NONCOPYABLE_H_
#define WOODYCXX_BASIC_NONCOPYABLE_H_

namespace woodycxx {

namespace noncopyable_  // protection from unintended ADL
{
class noncopyable {
 protected:
  noncopyable() {}
  ~noncopyable() {}

 private:  // emphasize the following members are private
  noncopyable(const noncopyable &);
  noncopyable &operator=(const noncopyable &);
};

}

typedef noncopyable_::noncopyable noncopyable;
}

#endif