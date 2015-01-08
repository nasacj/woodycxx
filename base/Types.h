/*
 * Types.h
 *
 *  Created on: 2014-7-14
 *      Author: qianchj
 */

#ifndef TYPES_H_
#define TYPES_H_

namespace woodycxx { namespace error {
enum ERROR_NUM
{
    NO_ERROR                  = 0x0,
    
    //Common Errors
    IndexOutOfBounds          = 0xF0000001,
    NullPointerError,
    InvalidPrameter,
    BufferIndexOutOfBounds,

    SocketError               = 0xF8000001,
    ConnectionReset           = 0xF8000002,
    InvalidIpAddress,
    ConnectionError
};

class IO_ERROR_CODE
{
public:
    static const int No_Error           = 0x0;
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