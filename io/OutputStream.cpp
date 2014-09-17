#include "OutputStream.h"

namespace woodycxx { namespace io {


void OutputStream::write(ByteBuffer& b)
{
    write(b, 0, b.getSize());
}

void OutputStream::write(const void* b, std::size_t len)
{
    write(b, len, 0, len);
}

void OutputStream::write(ByteBuffer& b, int off, int len)
{
    if ( 0 == b.getSize() )
        return;
    else if ( (off < 0) || (off > b.getSize()) || (len < 0) ||
        ( (off + len) > b.getSize()) || ((off + len) < 0)) {
            return; //TODO: How to handle this error
    } else if (len == 0) {
        return;
    }
    for (int i = 0 ; i < len ; i++) {
        write(b[off + i]);
    }
}

void OutputStream::write(const void* b, std::size_t b_size, std::size_t off, std::size_t len)
{
    if ( 0 == b )
        return;
    else if ( (off < 0) || (off > b_size) || (len < 0) ||
        ( (off + len) > b_size) || ((off + len) < 0)) {
            return; //TODO: How to handle this error
    } else if (len == 0) {
        return;
    }
    for (int i = 0 ; i < len ; i++) {
        write((static_cast<const uint8*>(b))[off + i]);
    }
}


}}