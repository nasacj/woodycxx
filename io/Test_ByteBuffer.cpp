/*
  Copyright (c) 2014-2015 by NASa Qian <nasacj@nasacj.net>
  This file is part of the woodycxx library.

  This software is distributed under BSD 3-Clause License.
  The full license agreement can be found in the LICENSE file.

  This software is distributed without any warranty.
*/

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

    uint8 *a = new uint8[10];
    buf.copy(a, 10);
    std::cout << std::endl << "After buf.copy(a, 10); ===>" << std::endl;
    for (int i = 0; i < 10; i++)
    {
        std::cout << static_cast<int>(a[i]) << " ";
    }

    ByteBuffer buf2(5);
    ByteBuffer buf3(3);
    for (int i = 0; i < buf3.getSize(); i++)
    {
        buf3[i] = i;
    }
    buf3.copy(buf2);
    std::cout << std::endl << "After buf3.copy(buf2); ===>" << std::endl;
    for (int i = 0; i < buf2.getSize(); i++)
    {
        std::cout << static_cast<int>(buf2[i]) << " ";
    }

    return 0;
}
#endif
