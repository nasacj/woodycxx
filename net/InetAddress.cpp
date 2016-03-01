/*
  Copyright (c) 2014-2015 by NASa Qian <nasacj@nasacj.net>
  This file is part of the woodycxx library.

  This software is distributed under BSD 3-Clause License.
  The full license agreement can be found in the LICENSE file.

  This software is distributed without any warranty.
*/

#include "InetAddress.h"
#include "SocketsOpt.h"
#include <stdint.h>
#include <assert.h>
#ifdef WIN32
#include <WinSock2.h>
#include <ws2tcpip.h>
#define bzero(x,y) ZeroMemory(x,y)
typedef uint32_t in_addr_t;
#else
#include <strings.h>   //bzero
#include <sys/socket.h>
#include <netdb.h>
#endif
#if defined(__CYGWIN__)
#include <string.h>   //bzero for cygwin
#endif

using namespace woodycxx::net;

InetAddress::InetAddress(uint16_t port, bool loopbackOnly)
{
	//TODO: localhost is not IPv6
	isIPv6 = false;
    bzero(&addr_, sizeof addr_);
    addr_.sin_family = AF_INET;
    in_addr_t ip = loopbackOnly ? INADDR_LOOPBACK : INADDR_ANY;
    addr_.sin_addr.s_addr = htonl(ip);
    addr_.sin_port = htons(port);
}

InetAddress::InetAddress(string ip, uint16_t port)
{
	if (string::npos != ip.find(":"))
	{
		bzero(&addr6_, sizeof addr6_);
		sockets::fromIpPort(ip.c_str(), port, &addr6_);
		isIPv6 = true;
	}
	else
	{
		bzero(&addr_, sizeof addr_);
		sockets::fromIpPort(ip.c_str(), port, &addr_);
		isIPv6 = false;
	}
}

InetAddress::InetAddress(const string& host, const struct in6_addr& address)
{
	bzero(&(this->sin_addr6), sizeof(this->sin_addr6));
	this->originalHostName = host;
	this->hostName = host;
	this->sin_addr6 = address;
	this->family = AF_INET6;
}

InetAddress::InetAddress(const string& host, const struct in_addr& address)
{
	bzero(&(this->sin_addr6), sizeof(this->sin_addr6));
	this->originalHostName = host;
	this->hostName = host;
	this->sin_addr = address;
	this->family = AF_INET;
}

list<InetAddress> InetAddress::getAllByNameIPv4(const string& host)
{
	string errMsg;
	list<InetAddress> addr_list;
	struct hostent	*remoteHost;
	string comm_err_msg = string("Failed to get Host Name: ") + host + ", ";
	if ((remoteHost = ::gethostbyname(host.c_str())) == NULL)
	{
#ifdef WIN32
		DWORD dwError = WSAGetLastError();
		string win_msg = "";
		if (dwError != 0) {
			if (dwError == WSAHOST_NOT_FOUND) {
				errMsg = comm_err_msg + "Host not found";
			}
			else if (dwError == WSANO_DATA) {
				errMsg = comm_err_msg + "No data record found";
			}
			else {
				errMsg = comm_err_msg + "Function failed with error";
			}
		}
#else
		errMsg = comm_err_msg + string(hstrerror(h_errno));
#endif
		throw UnknownHostException(errMsg);
	}
	else
	{
		int i = 0;
		string official_hostname = remoteHost->h_name;
		switch (remoteHost->h_addrtype) {
		case AF_INET:
			while (remoteHost->h_addr_list[i] != 0)
			{
				struct in_addr addr;
				addr.s_addr = *(u_long *)remoteHost->h_addr_list[i++];
				InetAddress inetAdd(official_hostname, addr);
				addr_list.push_back(inetAdd);
			}
			break;

		default:
			errMsg = comm_err_msg + "unknown address type";
			break;
		}

	}
	return addr_list;
}

list<InetAddress> InetAddress::getAllByName(const string& host)
{
	string errMsg;
	list<InetAddress> addr_list;
	string comm_err_msg = string("Failed to get Host Name: ") + host + ", ";

	struct addrinfo *result = NULL;
	struct addrinfo *ptr = NULL;
	struct addrinfo hints;

	struct sockaddr_in  *sockaddr_ipv4;
	struct sockaddr_in6 *sockaddr_ipv6;

	bzero(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	int dwRetval = ::getaddrinfo(host.c_str(), "", &hints, &result);
	if (dwRetval != 0)
	{
#ifdef _WINDOWS_
		WCHAR* msg = gai_strerror(dwRetval);
		DWORD num = WideCharToMultiByte(CP_ACP, 0, msg, -1, NULL, 0, NULL, 0);
		char *cword = new char[num];
		bzero(cword, num);
		WideCharToMultiByte(CP_ACP, 0, msg, -1, cword, num, NULL, 0);
		errMsg = comm_err_msg + string(cword);
		delete[](cword);
#else
		errMsg = comm_err_msg + gai_strerror(dwRetval);
#endif
		throw UnknownHostException(errMsg);
	}
	else
	{
		for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
		{
			switch (ptr->ai_family) {
			case AF_INET:
			{
				sockaddr_ipv4 = (struct sockaddr_in *) ptr->ai_addr;
				struct in_addr addr;
				addr = sockaddr_ipv4->sin_addr;
				InetAddress inetAdd(host, addr);
				addr_list.push_back(inetAdd);
				break;
			}
			case AF_INET6:
			{
				sockaddr_ipv6 = (struct sockaddr_in6 *) ptr->ai_addr;
				struct in6_addr addr6;
				addr6 = sockaddr_ipv6->sin6_addr;
				InetAddress inetAdd6(host, addr6);
				addr_list.push_back(inetAdd6);
				break;
			}
			}//end of switch
		}
		::freeaddrinfo(result);
	}
	return addr_list;
}

InetAddress InetAddress::getByName(const string& host)
{
	list<InetAddress> addr_list = getAllByName(host);
	if (!addr_list.empty())
		return addr_list.front();
	//TODO: UnknowhostException
	in_addr ad;
	bzero(&ad, sizeof(ad));
	InetAddress addr(host, ad);
	return addr;
}

InetAddress InetAddress::getByAddress(const struct in_addr& address)
{
	InetAddress inetAddr("", address);
	return inetAddr;
}

InetAddress InetAddress::getByAddress(const struct in6_addr& address)
{
	InetAddress inetAddr("", address);
	return inetAddr;
}

InetAddress InetAddress::getByAddress(const string& host, const struct in_addr& address)
{
	InetAddress inetAddr(host, address);
	return inetAddr;
}

InetAddress InetAddress::getByAddress(const string& host, const struct in6_addr& address)
{
	InetAddress inetAddr(host, address);
	return inetAddr;
}


const struct sockaddr* InetAddress::getSockAddrInet() const
{ 
	return sockets::sockaddr_cast(&addr6_);
}

string InetAddress::getIpPort() const
{
    char buf[INET6_ADDRSTRLEN];
    sockets::toIpPort(buf, sizeof buf, (const sockaddr*)&addr6_);
    return buf;
}

string InetAddress::getIp() const
{
    char buf[INET6_ADDRSTRLEN];
    sockets::toIp(buf, sizeof buf, (const sockaddr*)&addr6_);
    return buf;
}

uint16_t InetAddress::getPort() const
{
    return ntohs(addr_.sin_port);
}

bool InetAddress::isIPV6() const
{
	return this->isIPv6;
}

string InetAddress::getHostByAddr(const InetAddress& inet_address)
{
	char hostname[1025] = { 0 };
	char servInfo[32] = { 0 };
	int dwRetval = 0;
	socklen_t size = 0;
	struct sockaddr * sa = NULL;
	struct sockaddr_in sockaddr_in_ipv4;
	struct sockaddr_in6 sockaddr_in_ipv6;
	bzero(&sockaddr_in_ipv4, sizeof(sockaddr_in_ipv4));
	bzero(&sockaddr_in_ipv6, sizeof(sockaddr_in_ipv6));
	switch (inet_address.family) {
	case AF_INET:
	{
		sockaddr_in_ipv4.sin_family = inet_address.family;
		sockaddr_in_ipv4.sin_addr = inet_address.sin_addr;
		sockaddr_in_ipv4.sin_port = htons(0);
		sa = (struct sockaddr *)&sockaddr_in_ipv4;
		size = sizeof(struct sockaddr_in);
		break;
	}
	case AF_INET6:
	{
		sockaddr_in_ipv6.sin6_family = inet_address.family;
		sockaddr_in_ipv6.sin6_addr = inet_address.sin_addr6;
		sockaddr_in_ipv6.sin6_port = htons(0);
		sa = (struct sockaddr *)&sockaddr_in_ipv6;
		size = sizeof(struct sockaddr_in6);
		break;
	}
	}//end of switch
	dwRetval = getnameinfo(sa, size,
		hostname, sizeof(hostname),
		servInfo, sizeof(servInfo), NI_NUMERICSERV);
	return string(hostname);
}

InetAddress InetAddress::getLoopbackAddress()
{
	struct in_addr ipv4Addr;
	bzero(&ipv4Addr, sizeof(ipv4Addr));
	in_addr_t ip = INADDR_LOOPBACK;
	ipv4Addr.s_addr = htonl(ip);
	return InetAddress("localhost", ipv4Addr);
}

InetAddress InetAddress::getLoopbackAddressIPv6()
{
	return InetAddress("localhost", in6addr_loopback);
}

InetAddress InetAddress::getAnylocalAddress()
{
	struct in_addr ipv4Addr;
	in_addr_t ip = INADDR_ANY;
	ipv4Addr.s_addr = htonl(ip);
	return InetAddress(getLocalHostName(), ipv4Addr);
}

InetAddress InetAddress::getAnylocalAddressIPv6()
{
	return InetAddress(getLocalHostName(), in6addr_any);
}

string InetAddress::getHostName() const
{
	if ( (this->hostName != "") && (this->hostName.length() > 0) )
		return this->hostName;
	return getHostByAddr(*this);
}

string InetAddress::getHostAddress() const
{
	char buf[INET6_ADDRSTRLEN] = { 0 };
	sockets::toIp( buf, sizeof(buf), (void*)&(this->sin_addr6), this->family );
	return string(buf);
}

string InetAddress::getLocalHostName()
{
	char hostname[256] = {};
	::gethostname(hostname, sizeof(hostname));
	return string(hostname);
}

void InetAddress::setHostName(const string& hname)
{
	this->hostName = hname;
}

bool InetAddress::resolve(string hostname, InetAddress* result)
{
    /*
    assert(out != NULL);
    struct hostent hent;
    struct hostent* he = NULL;
    int herrno = 0;
    bzero(&hent, sizeof(hent));

    int ret = gethostbyname_r(hostname.c_str(), &hent, t_resolveBuffer, sizeof t_resolveBuffer, &he, &herrno);
    if (ret == 0 && he != NULL)
    {
        assert(he->h_addrtype == AF_INET && he->h_length == sizeof(uint32_t));
        out->addr_.sin_addr = *reinterpret_cast<struct in_addr*>(he->h_addr);
        return true;
    }
    else
    {
        if (ret)
        {
            LOG_SYSERR << "InetAddress::resolve";
        }
        return false;
    }
    */
    return false;
}
