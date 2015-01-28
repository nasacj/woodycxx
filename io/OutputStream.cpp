/*
  Copyright (c) 2014-2015 by NASa Qian <nasacj@nasacj.net>
  This file is part of the woodycxx library.

  This software is distributed under BSD 3-Clause License.
  The full license agreement can be found in the LICENSE file.

  This software is distributed without any warranty.
*/

#include "OutputStream.h"

namespace woodycxx { namespace io {


int OutputStream::write(ByteBuffer& b)
{
    return write(b, 0, b.getSize());
}

int OutputStream::write(const void* b, int len)
{
    return write(b, len, 0, len);
}

int OutputStream::write(ByteBuffer& b, int off, int len)
{
    int write_count = 0;
    if ( 0 == b.getSize() )
        return woodycxx::error::NullPointerError;
    else if ( (off < 0) || (off > b.getSize()) || (len < 0) ||
        ( (off + len) > b.getSize()) || ((off + len) < 0)) {
        return woodycxx::error::BufferIndexOutOfBounds;
    } else if (len == 0) {
        return 0;
    }
    for (int i = 0 ; i < len ; i++) {
        int temp_count = write(b[off + i]);
        if ( temp_count >= 0 )
            write_count += temp_count;
        else
            return temp_count;
    }
    return write_count;
}

int OutputStream::write(const void* b, int b_size, int off, int len)
{
    if ( 0 == b )
        return woodycxx::error::NullPointerError;
    else if ( (off < 0) || (off > b_size) || (len < 0) ||
        ( (off + len) > b_size) || ((off + len) < 0)) {
            return woodycxx::error::BufferIndexOutOfBounds;
    } else if (len == 0) {
        return 0;
    }

    int write_count = 0;
    for (int i = 0 ; i < len ; i++)
    {
        int temp_count = write((static_cast<const char*>(b))[off + i]);
        if ( temp_count >= 0 )
            write_count += temp_count;
        else
            return temp_count;
    }
    return write_count;
}


}}