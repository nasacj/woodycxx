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

int InputStream::read(void* b, std::size_t len)
{
    return read(b, len, 0, len);
}

int InputStream::read(ByteBuffer& b, int off, int len)
{
    if ( 0 == b.getSize() )
        return IO_ERROR_CODE::NullPointerError;
    else if (off < 0 || len < 0 || len > b.getSize() - off)
        return IO_ERROR_CODE::InvalidPrameter;
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

int InputStream::read(void* b, std::size_t buf_size, std::size_t off, std::size_t len)
{
    if ( 0 == b )
        return IO_ERROR_CODE::NullPointerError;
    else if (off < 0 || len < 0 || len > buf_size - off)
        return IO_ERROR_CODE::InvalidPrameter;
    else if (len == 0)
        return 0;

    int c = read();
    if ( c == -1 ) {
        return -1;
    }
    (static_cast<uint8*>(b))[off] = static_cast<uint8>(c);

    int i = 1;
    for (; i < len ; i++)
    {
        c = read();
        if (c == -1)
            break;
        (static_cast<uint8*>(b))[off + i] = static_cast<uint8>(c);
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
    typedef woodycxx::smart_prt::scoped_array<uint8> Byte_Array;
    uint8* skipBuffer = Byte_Array(new uint8[size]).get();
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
