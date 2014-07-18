/*
 * Types.h
 *
 *  Created on: 2014-7-14
 *      Author: qianchj
 */

#ifndef TYPES_H_
#define TYPES_H_

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

typedef signed char         int8;
typedef short               int16;
typedef int                 int32;
typedef long long           int64;

typedef unsigned char      uint8;
typedef unsigned short     uint16;
typedef unsigned int       uint32;
typedef unsigned long long uint64;

#endif /* TYPES_H_ */