/*
  Copyright (c) 2014-2015 by NASa Qian <nasacj@nasacj.net>
  This file is part of the woodycxx library.

  This software is distributed under BSD 3-Clause License.
  The full license agreement can be found in the LICENSE file.

  This software is distributed without any warranty.
*/

#ifndef WOODYCXX_SMART_PTR_SP_COUNTED_BASE_H_
#define WOODYCXX_SMART_PTR_SP_COUNTED_BASE_H_

namespace woodycxx {
namespace smart_prt {
namespace detail {

class sp_counted_base {
 private:

  sp_counted_base(sp_counted_base const &);
  sp_counted_base &operator=(sp_counted_base const &);

  int use_count_;        // #shared
  int weak_count_;       // #weak + (#shared != 0)

 public:

  sp_counted_base() : use_count_(1), weak_count_(1) {
  }

  virtual ~sp_counted_base() {
  }

  // dispose() is called when use_count_ drops to zero, to release
  // the resources managed by *this.
  virtual void dispose() = 0; // nothrow

  // destroy() is called when weak_count_ drops to zero.
  virtual void destroy() // nothrow
  {
    delete this;
  }

  void add_ref_copy() {
    //atomic_increment( &use_count_ );
    ++use_count_;
  }

  void release() // nothrow
  {
    if (--use_count_ == 0) {
      dispose();
      destroy();
    }
  }

  long use_count() const // nothrow
  {
    return static_cast<int const volatile &>( use_count_ );
  }
};

}
}
}

#endif
