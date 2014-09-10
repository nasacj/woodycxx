#include <iostream>
#include "ByteBuffer.h"

using woodycxx::io::ByteBuffer;

#if 1

int main()
{
    ByteBuffer buf(10);
    for (int i = 0; i < buf.getSize(); i++)
    {
        buf[i] = i;
    }

    for (int i = 0; i < buf.getSize(); i++)
    {
        std::cout << static_cast<int>(buf[i]) << " ";
    }
    return 0;
}
#endif
