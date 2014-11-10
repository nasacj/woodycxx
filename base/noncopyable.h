/*
 *  noncopyable.h
 *
 *  Created on: 2014-11-10
 *      Author: qianchj
 */

#ifndef WOODYCXX_BASIC_NONCOPYABLE_H_
#define WOODYCXX_BASIC_NONCOPYABLE_H_

namespace woodycxx {

namespace noncopyable_  // protection from unintended ADL
{
    class noncopyable
    {
    protected:
        noncopyable() {}
        ~noncopyable() {}

    private:  // emphasize the following members are private
        noncopyable( const noncopyable& );
        noncopyable& operator=( const noncopyable& );
    };

}

typedef noncopyable_::noncopyable noncopyable;
}

#endif