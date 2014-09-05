#include "OutputStream.h"

namespace woodycxx { namespace io {


void OutputStream::write(ByteBuffer& b)
{
    write(b, 0, b.getSize());
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


}}