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

using namespace woodycxx::net;
using namespace std;

int main()
{
    InetAddress address1(80);
    InetAddress address2("127.0.0.1", 12345);
    InetAddress address3("127.0.k.1", 123);
    InetAddress address4("fe80::ccae:f2cd:f261:fa4d", 12345);
    InetAddress address5("::1", 123);
    cout << "address1.getIpPort() = " << address1.getIpPort() << endl;
    cout << "address2.getIpPort() = " << address2.getIpPort() << endl;
    cout << "address3.getIpPort() = " << address3.getIpPort() << endl;
    cout << "address4.getIpPort() = " << address4.getIpPort() << endl;
    cout << "address5.getIpPort() = " << address5.getIpPort() << endl;
    cout << "address1.getIp() = " << address1.getIp() << endl;
    cout << "address2.getIp() = " << address2.getIp() << endl;
    cout << "address3.getIp() = " << address3.getIp() << endl;
    cout << "address4.getIp() = " << address4.getIp() << endl;
    cout << "address5.getIp() = " << address5.getIp() << endl;
    cout << "address1.getPort() = " << address1.getPort() << endl;
    cout << "address2.getPort() = " << address2.getPort() << endl;
	cout << "address3.getPort() = " << address3.getPort() << endl;
	cout << "address4.getPort() = " << address4.getPort() << endl;
	cout << "address5.getPort() = " << address5.getPort() << endl;
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

		cout << InetAddress::getLoopbackAddress().getHostName() << " = " << InetAddress::getLoopbackAddress().getHostAddress() << endl;
		cout << InetAddress::getLoopbackAddressIPv6().getHostName() << " = " << InetAddress::getLoopbackAddressIPv6().getHostAddress() << endl;
		cout << InetAddress::getAnylocalAddress().getHostName() << " = " << InetAddress::getAnylocalAddress().getHostAddress() << endl;
		cout << InetAddress::getAnylocalAddressIPv6().getHostName() << " = " << InetAddress::getAnylocalAddressIPv6().getHostAddress() << endl;

		cout << "Is 2 InetAddress are equal (true)---> " << boolalpha << (InetAddress::getLoopbackAddress() == InetAddress::getLoopbackAddress()) << endl;
		cout << "Is 2 InetAddress are equal (false)---> " << boolalpha << (InetAddress::getLoopbackAddress() == InetAddress::getAnylocalAddress()) << endl;
	}
	catch (Exception& unkonwhostexp)
	{
		cout << "catch <Exception> --> " << unkonwhostexp.what() << endl;
	}
    
    return 0;
}
