/*
  Copyright (c) 2014-2015 by NASa Qian <nasacj@nasacj.net>
  This file is part of the woodycxx library.

  This software is distributed under BSD 3-Clause License.
  The full license agreement can be found in the LICENSE file.

  This software is distributed without any warranty.
*/

#include <iostream>
#include "InetSocketAddress.h"
#include <assert.h>

using namespace woodycxx::net;
using namespace std;

#if 1

int main()
{
	try
	{
		InetSocketAddress address("baidu.com", 8000);
		cout << address.toString() << endl;

		InetSocketAddress address2("baidu.com", 8000);
		assert(address == address2);

		InetSocketAddress address3(80000000);
	}
	catch (Exception& e)
	{
		cout << "Catch Exception --> " << e.what() << endl;
	}
    


    return 0;
}

#endif