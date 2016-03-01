/*
  Copyright (c) 2014-2015 by NASa Qian <nasacj@nasacj.net>
  This file is part of the woodycxx library.

  This software is distributed under BSD 3-Clause License.
  The full license agreement can be found in the LICENSE file.

  This software is distributed without any warranty.
*/

#ifndef Woodycxx_Net_InetAddress_H_
#define Woodycxx_Net_InetAddress_H_

#include <string>
#include <stdint.h>
#include <sys/types.h>
#include <list>
#include "UnknownHostException.h"
#ifdef WIN32
#include <WinSock2.h>
#include <ws2ipdef.h>
#else
#include <netinet/in.h>
#endif

using namespace std;

namespace woodycxx { namespace net {

class InetAddress
{
protected:
	explicit InetAddress(const string& host, const struct in6_addr& address);

	explicit InetAddress(const string& host, const struct in_addr& address);

public:
    /// Constructs an endpoint with given port number.
    explicit InetAddress(uint16_t port, bool loopbackonly = false);

    /// Constructs an endpoint with given ip and port.
	InetAddress(string ip, uint16_t port);

    // Constructs an endpoint with given struct @c sockaddr_in
    InetAddress(const struct sockaddr_in& addr)
        : addr_(addr)
    { }

	string getIp() const;
	string getIpPort() const;
	uint16_t getPort() const;
	bool isIPV6() const;

	// resolve hostname to IP address
	// return true on success.
	// thread safe
	static bool resolve(string hostname, InetAddress* result);

	//-----------------------------------------------------------------------------

	const struct sockaddr* getSockAddrInet() const;
	void setSockAddrInet(const struct sockaddr_in& addr) { addr_ = addr; }

	string getHostName() const;
	string getHostAddress() const;
	void setHostName(const string& hname);

	//TODO: Use cache, then it will not call gethostbyname every time.
	static list<InetAddress> getAllByNameIPv4(const string& host);
	static list<InetAddress> getAllByName(const string& host);
	static InetAddress getByName(const string& host);
	static InetAddress getByAddress(const struct in_addr& address);
	static InetAddress getByAddress(const struct in6_addr& address);
	static InetAddress getByAddress(const string& host, const struct in_addr& address);
	static InetAddress getByAddress(const string& host, const struct in6_addr& address);
	static InetAddress getLoopbackAddress();
	static InetAddress getLoopbackAddressIPv6();
	static InetAddress getAnylocalAddress();
	static InetAddress getAnylocalAddressIPv6();
	static string getLocalHostName();

private:
	static string getHostByAddr(const InetAddress& inet_address);

	union 
	{
		struct sockaddr_in addr_;
		struct sockaddr_in6 addr6_;
	};
	union
	{
		struct in_addr sin_addr;
		struct in6_addr sin_addr6;
	};
	bool isIPv6;
	string originalHostName;
	string hostName;
	uint16_t family;
};

}}//end of namespace woodycxx::net

#endif
