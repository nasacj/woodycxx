/*
 * Math.h
 *
 *  Created on: 2014-7-18
 *      Author: qianchj
 */

#ifndef MATH_H_
#define MATH_H_

namespace woodycxx { namespace io {

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