/*
  Copyright (c) 2014-2015 by NASa Qian <nasacj@nasacj.net>
  This file is part of the woodycxx library.

  This software is distributed under BSD 3-Clause License.
  The full license agreement can be found in the LICENSE file.

  This software is distributed without any warranty.
*/

#ifndef WOODYCXX_SMART_PTR_SCOPED_ARRAY_H_
#define WOODYCXX_SMART_PTR_SCOPED_ARRAY_H_

#include "checked_delete.h"
#include <cstddef>            // for std::ptrdiff_t
#include <assert.h>

namespace woodycxx {
namespace smart_prt {

//  scoped_array extends scoped_ptr to arrays. Deletion of the array pointed to
//  is guaranteed, either on destruction of the scoped_array or via an explicit
//  reset(). Use shared_array or std::vector if your needs are more complex.

template<class T>
class scoped_array // noncopyable
{
 private:

  T *px;

  scoped_array(scoped_array const &);
  scoped_array &operator=(scoped_array const &);

  typedef scoped_array<T> this_type;

  void operator==(scoped_array const &) const;
  void operator!=(scoped_array const &) const;

 public:

  typedef T element_type;

  explicit scoped_array(T *p = 0) : px(p) {
  }

  ~scoped_array() // never throws
  {
    checked_delete(px);
  }

  void reset(T *p = 0) // never throws
  {
    assert(p == 0 || p != px); // catch self-reset errors
    this_type(p).swap(*this);
  }

  T &operator[](std::ptrdiff_t i) const {
    assert(px != 0);
    assert(i >= 0);
    return px[i];
  }

  T *get() const {
    return px;
  }

  void swap(scoped_array &b) {
    T *tmp = b.px;
    b.px = px;
    px = tmp;
  }
};

}
}
#endif
