/*
  Copyright (c) 2014-2015 by NASa Qian <nasacj@nasacj.net>
  This file is part of the woodycxx library.

  This software is distributed under BSD 3-Clause License.
  The full license agreement can be found in the LICENSE file.

  This software is distributed without any warranty.
*/

#include "InetAddress.h"
#include <iostream>
#include <list>
#include <assert.h>

using namespace woodycxx::net;
using namespace std;

int main()
{
	cout << "localhost name = " << InetAddress::getLocalHostName() << endl;
	cout << "baidu.com IP = " << InetAddress::getByName("baidu.com").getHostAddress() << endl;

	try
	{
		list<InetAddress> addrList = InetAddress::getAllByNameIPv4("baidu.com");
		for (InetAddress address : addrList)
			cout << address.getHostName() << " --> " << address.getHostAddress() << endl;

		cout << "-----------------getAllByName(localhost)------------------" << endl; 
		list<InetAddress> addrList2 = InetAddress::getAllByName("localhost");
		for (InetAddress address : addrList2)
			cout << address.getHostName() << " --> " << address.getHostAddress() << endl;
		
		cout << "-----------------getAllByNameIPv4(localhost)------------------" << endl; 
		list<InetAddress> addrList3 = InetAddress::getAllByNameIPv4("localhost");
		for (InetAddress address : addrList3)
			cout << address.getHostName() << " --> " << address.getHostAddress() << endl;

		cout << "--------getAllByName(addr_localhost.getHostName())---------" << endl; 
		InetAddress addr_localhost = addrList3.front(); 
		list<InetAddress> addrList4 = InetAddress::getAllByName(addr_localhost.getHostName());
		for (InetAddress address : addrList4)
			cout << address.getHostName() << " --> " << address.getHostAddress() << endl;
		
		//cout << "--------getAllByName(hahaha)---------" << endl; 
		//list<InetAddress> addrList5 = InetAddress::getAllByName("hahaha");
		//for (InetAddress address : addrList5)
		//	cout << address.getHostName() << " --> " << address.getHostAddress() << endl;

		cout << "--------getLoopbackAddress & getAnylocalAddress---------" << endl;
		cout << InetAddress::getLoopbackAddress().getHostName() << " = " << InetAddress::getLoopbackAddress().getHostAddress() << endl;
		cout << InetAddress::getLoopbackAddressIPv6().getHostName() << " = " << InetAddress::getLoopbackAddressIPv6().getHostAddress() << endl;
		cout << InetAddress::getAnylocalAddress().getHostName() << " = " << InetAddress::getAnylocalAddress().getHostAddress() << endl;
		cout << InetAddress::getAnylocalAddressIPv6().getHostName() << " = " << InetAddress::getAnylocalAddressIPv6().getHostAddress() << endl;

		assert(InetAddress::getLoopbackAddress() == InetAddress::getLoopbackAddress());
		assert(InetAddress::getLoopbackAddress() != InetAddress::getAnylocalAddress());

	}
	catch (Exception& unkonwhostexp)
	{
		cout << "catch <Exception> --> " << unkonwhostexp.what() << endl;
	}
    
    return 0;
}
