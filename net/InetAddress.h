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

    const struct sockaddr* getSockAddrInet() const;
    void setSockAddrInet(const struct sockaddr_in& addr) { addr_ = addr; }

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
    
};

}}//end of namespace woodycxx::net

#endif
