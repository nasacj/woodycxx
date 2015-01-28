/*
  Copyright (c) 2014-2015 by NASa Qian <nasacj@nasacj.net>
  This file is part of the woodycxx library.

  This software is distributed under BSD 3-Clause License.
  The full license agreement can be found in the LICENSE file.

  This software is distributed without any warranty.
*/

#include <iostream>
#include "InetSocketAddress.h"

using namespace woodycxx::net;
using namespace std;

#if 1

int main()
{
    InetSocketAddress address("127.0.0.1", 8000);
    cout << "address, ip = " << address.getHostName() << ", port = " << address.getPort() << endl;

    InetSocketAddress address1("127.0.0.1", 8001);
    InetSocketAddress address2("127.0.0.1", 8000);
    InetSocketAddress address3("127.0.0.2", 8000);

    cout << "address == address1 ? " << (address == address1) << endl;
    cout << "address == address2 ? " << (address == address2) << endl;
    cout << "address == address3 ? " << (address == address3) << endl;

    return 0;
}

#endif