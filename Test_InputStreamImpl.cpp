#include <iostream>
#include "InputStreamImpl.h"

using namespace woodycxx::io;

#if 1

int main()
{
    ByteBuffer buf(10);
    InputStreamImpl inputstreamimpl;
    InputStream& inputstream = inputstreamimpl;
    inputstream.read(buf);

    for (int i = 0; i < buf.getSize(); i++)
    {
        std::cout << static_cast<int>(buf[i]) << " ";
    }

    InputStreamImpl impl;
    impl.InputStream::read(buf);
    //impl.read(buf);

    return 0;
}


#endif