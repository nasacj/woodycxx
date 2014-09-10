#include "InputStream.h"
#include "Math.h"
#include <iostream>

using namespace std;
using namespace woodycxx::base;

namespace woodycxx { namespace io {

int InputStream::read(ByteBuffer& b)
{
    return read(b, 0, b.getSize());
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

long InputStream::skip(long n)
{
    long remaining = n;
    int nr;

    if (n <= 0) {
        return 0;
    }

    int size = static_cast<int>(Math::min(static_cast<long>(MAX_SKIP_BUFFER_SIZE), remaining));
    ByteBuffer skipBuffer(size);
    while (remaining > 0)
    {
        nr = read( skipBuffer, 0, static_cast<int>( Math::min(static_cast<long>(size), remaining) ) );
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