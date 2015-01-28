/*
  Copyright (c) 2014-2015 by NASa Qian <nasacj@nasacj.net>
  This file is part of the woodycxx library.

  This software is distributed under BSD 3-Clause License.
  The full license agreement can be found in the LICENSE file.

  This software is distributed without any warranty.
*/

#include "InputStream.h"
#include <base/Math.h>
#include <smart_ptr/scoped_array.h>
#include <iostream>

using namespace std;
using namespace woodycxx::base;

namespace woodycxx { namespace io {

int InputStream::read(ByteBuffer& b)
{
    return read(b, 0, b.getSize());
}

int InputStream::read(char* b, int len)
{
    return read(b, len, 0, len);
}

int InputStream::read(ByteBuffer& b, int off, int len)
{
    if ( 0 == b.getSize() )
        return woodycxx::error::NullPointerError;
    else if (off < 0 || len < 0 || len > b.getSize() - off)
        return woodycxx::error::InvalidPrameter;
    else if (len == 0)
        return 0;

    int c = read();
    if ( c == -1 ) {
        return -1;
    }
    b[off] = static_cast<uint8>(c);

    int i = 1;
    for (; i < len ; i++)
    {
        c = read();
        if (c == -1)
            break;
        b[off + i] = static_cast<uint8>(c);
    }
    return i;

}

int InputStream::read(char* b, int buf_size, int off, int len)
{
    if ( 0 == b )
        return woodycxx::error::NullPointerError;
    else if (off < 0 || len < 0 || len > buf_size - off)
        return woodycxx::error::InvalidPrameter;
    else if (len == 0)
        return 0;

    int c = read();
    if ( c == -1 ) {
        return -1;
    }
    (static_cast<char*>(b))[off] = static_cast<uint8>(c);

    int i = 1;
    for (; i < len ; i++)
    {
        c = read();
        if (c == -1)
            break;
        (static_cast<char*>(b))[off + i] = static_cast<uint8>(c);
    }
    return i;

}

long InputStream::skip(long n)
{
    long remaining = n;
    int nr;

    if (n <= 0) {
        return 0;
    }

    int size = static_cast<int>(Math::min(static_cast<long>(MAX_SKIP_BUFFER_SIZE), remaining));
    //ByteBuffer skipBuffer(size);
    typedef woodycxx::smart_prt::scoped_array<char> Byte_Array;
    char* skipBuffer = Byte_Array(new char[size]).get();
    while (remaining > 0)
    {
        nr = read( skipBuffer, size, 0, static_cast<int>( Math::min(static_cast<long>(size), remaining) ) );
        if (nr < 0)
        {
            break;
        }
        remaining -= nr;
    }
    return n - remaining;
}

int InputStream::available()
{
    return 0;
}



void InputStream::hello()
{
    cout << "InputStream::hello" << endl;
}

}}
