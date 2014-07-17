#include "InputStream.h"

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

int InputStream::available()
{
    return 0;
}

}}