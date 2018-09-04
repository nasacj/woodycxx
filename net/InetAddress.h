/*
  Copyright (c) 2014-2015 by NASa Qian <nasacj@nasacj.net>
  This file is part of the woodycxx library.

  This software is distributed under BSD 3-Clause License.
  The full license agreement can be found in the LICENSE file.

  This software is distributed without any warranty.
*/

#ifndef Woodycxx_Net_InetAddress_H_
#define Woodycxx_Net_InetAddress_H_

#include <string.h>
#include <string>
#include <stdint.h>
#include <sys/types.h>
#include <list>
#include "UnknownHostException.h"
#ifdef WIN32
#include <WinSock2.h>
#include <ws2ipdef.h>
#define bzero(x,y) ZeroMemory(x,y)
typedef uint32_t in_addr_t;
#else
#include <netinet/in.h>
#endif
#if defined(__CYGWIN__)
#ifndef bzero
#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)
#endif
#endif

using namespace std;

namespace woodycxx { namespace net {

class InetAddress
{
protected:

	explicit InetAddress(const string& host, const struct in6_addr& address);

	explicit InetAddress(const string& host, const struct in_addr& address);

public:
	bool isIPV6() const;
	string toString() const;
	string getHostName() const;
	string getHostAddress() const;
	uint16_t getFamily() const;
	struct in_addr getAddress() const;
	struct in6_addr getAddressIPv6() const;
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

	friend bool operator==(const InetAddress& p1, const InetAddress& p2)
	{
		return ( (p1.family == p2.family) &&
			(p1.hostName == p2.hostName) &&
			(!memcmp(&p1.sin_addr6, &p2.sin_addr6, sizeof(p1.sin_addr6))));
	}

	friend bool operator!=(const InetAddress& p1, const InetAddress& p2)
	{
		return ((p1.family != p2.family) ||
			(p1.hostName != p2.hostName) ||
			( 0 != memcmp(&p1.sin_addr6, &p2.sin_addr6, sizeof(p1.sin_addr6))));
	}

private:
	static string getHostByAddr(const InetAddress& inet_address);

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
