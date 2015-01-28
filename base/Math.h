/*
  Copyright (c) 2014-2015 by NASa Qian <nasacj@nasacj.net>
  This file is part of the woodycxx library.

  This software is distributed under BSD 3-Clause License.
  The full license agreement can be found in the LICENSE file.

  This software is distributed without any warranty.
*/

#ifndef MATH_H_
#define MATH_H_

namespace woodycxx { namespace base {

class Math
{
public:
    static long min(long a, long b)
    {
        return (a <= b) ? a : b;
    }

    static int min(int a, int b)
    {
        return (a <= b) ? a : b;
    }
};


}}

#endif /* MATH_H_ */