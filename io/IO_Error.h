/*
 * IO_Error.h
 *
 *  Created on: 2014-9-10
 *      Author: qianchj
 */

#ifndef IO_ERROR_H_
#define IO_ERROR_H_

namespace woodycxx { namespace io {
enum ERROR_NUM
{
    NO_ERROR                  = 0x0,
    IndexOutOfBoundsException = 0xF0000001
};

class IO_ERROR_CODE
{
public:
    static const int No_Error           = 0x0;
    static const int NullPointerError   = 0xF0000001;
    static const int InvalidPrameter    = 0xF0000002;
};

}}

#endif /* IO_ERROR_H_ */