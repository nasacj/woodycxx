/*
  Copyright (c) 2014-2015 by NASa Qian <nasacj@nasacj.net>
  This file is part of the woodycxx library.

  This software is distributed under BSD 3-Clause License.
  The full license agreement can be found in the LICENSE file.

  This software is distributed without any warranty.
*/
#pragma once
#ifndef Woodycxx_Net_InetSocketAddress_H_
#define Woodycxx_Net_InetSocketAddress_H_

#include <string>
#include <stdint.h>
#include <sys/types.h>
#include <list>
#include <base/IllegalArgumentException.h>
#include "UnknownHostException.h"
#include "InetAddress.h"
#ifdef WIN32
#include <WinSock2.h>
#include <ws2ipdef.h>
#else
#include <netinet/in.h>
#include <sys/socket.h>
#endif

using namespace std;

namespace woodycxx {
namespace net {

class InetSocketAddress {
 public:

  InetSocketAddress(int tPort);

  InetSocketAddress(const InetAddress &addr, int tProt);

  InetSocketAddress(const string &host, int tProt);

  string getHostName();

  uint16_t getPort();

  InetAddress getAddress();

  uint16_t getFamily();

  string getHostAddress();

  string toString();

  bool isUnresolved();

  bool isIPv6();

  const struct sockaddr *const getSockAddrP() const;

  friend bool operator==(const InetSocketAddress &p1, const InetSocketAddress &p2) {
    return ((p1.inetAddress == p2.inetAddress) &&
        (p1.port == p2.port));
  }

  friend bool operator!=(const InetSocketAddress &p1, const InetSocketAddress &p2) {
    return ((p1.inetAddress != p2.inetAddress) ||
        (p1.port != p2.port));
  }

  ::sockaddr& as_posix_sockaddr() { return sa_; }
  ::sockaddr_in& as_posix_sockaddr_in() { return addr_; }
  const ::sockaddr& as_posix_sockaddr() const { return sa_; }
  const ::sockaddr_in& as_posix_sockaddr_in() const { return addr_; }

 private:
  void init();
  static int checkPort(int port);
  static string checkHost(const string &hostname);

  InetAddress inetAddress;
  uint16_t port;
  string hostname;
  bool Unresolved;

  union {
    struct sockaddr_in addr_;
    struct sockaddr_in6 addr6_;
    struct sockaddr sa_;
  };

};

}
}// end of namespace woodycxx::net

#endif
