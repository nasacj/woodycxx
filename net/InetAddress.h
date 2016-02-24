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
	class InetAddressHolder {
		/**
		* Reserve the original application specified hostname.
		*
		* The original hostname is useful for domain-based endpoint
		* identification (see RFC 2818 and RFC 6125).  If an address
		* was created with a raw IP address, a reverse name lookup
		* may introduce endpoint identification security issue via
		* DNS forging.
		*
		* Note: May define a new public method in the future if necessary.
		*/
		string originalHostName;
		string hostName;
		uint16_t family;
		union address_in
		{
			struct sockaddr_in addr_;
			struct sockaddr_in6 addr6_;
		};
	public:
		InetAddressHolder() {}
		InetAddressHolder(string hostName, uint16_t family) {
			this->originalHostName = hostName;
			this->hostName = hostName;
			this->family = family;

		}
	};

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

	//TODO: Use cache, then it will not call gethostbyname every time.
	static list<InetAddress> getAllByNameIPv4(const string& host, string& errMsg);
	static list<InetAddress> getAllByName(const string& host, string& errMsg);
	static InetAddress getByName(const string& host);
	static InetAddress getByAddress(const struct in_addr& address);
	static InetAddress getByAddress(const struct in6_addr& address);
	static InetAddress getByAddress(const string& host, const struct in_addr& address);
	static InetAddress getByAddress(const string& host, const struct in6_addr& address);

    string getIp() const;
    string getIpPort() const;
    uint16_t getPort() const;
	bool isIPV6() const;

	string getHostName() const;
	string getHostAddress() const;

    const struct sockaddr* getSockAddrInet() const;
    void setSockAddrInet(const struct sockaddr_in& addr) { addr_ = addr; }
	static string getLocalHost();
    // resolve hostname to IP address
    // return true on success.
    // thread safe
    static bool resolve(string hostname, InetAddress* result);

private:
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
