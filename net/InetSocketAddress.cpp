/*
Copyright (c) 2014-2015 by NASa Qian <nasacj@nasacj.net>
This file is part of the woodycxx library.

This software is distributed under BSD 3-Clause License.
The full license agreement can be found in the LICENSE file.

This software is distributed without any warranty.
*/

#include "InetSocketAddress.h"
#include "SocketsOpt.h"
#include <sstream>

using namespace woodycxx::base;
using namespace woodycxx::net;

void InetSocketAddress::init() {
  this->Unresolved = false;
  this->hostname = inetAddress.getHostName();
  if (inetAddress.getFamily() == AF_INET6) {
    bzero(&(this->addr6_), sizeof(struct sockaddr_in6));
    sockets::fromAddrPort(inetAddress.getAddressIPv6(), port, &addr6_);
  }
  if (inetAddress.getFamily() == AF_INET) {
    bzero(&(this->addr_), sizeof(struct sockaddr_in));
    sockets::fromAddrPort(inetAddress.getAddress(), port, &addr_);
  }
}

InetSocketAddress::InetSocketAddress(int tPort) :
    inetAddress(InetAddress::getAnylocalAddress()),
    port(checkPort(tPort)) {
  init();
}

InetSocketAddress::InetSocketAddress(const InetAddress &addr, int tPort) :
    inetAddress(addr),
    port(checkPort(tPort)) {
  init();
}

InetSocketAddress::InetSocketAddress(const string &host, int tProt) :
    inetAddress(InetAddress::getLoopbackAddress()),
    port(checkPort(tProt)) {
  checkHost(host);
  try {
    this->inetAddress = InetAddress::getByName(host);
    init();
  }
  catch (const UnknownHostException &) {
    this->Unresolved = true;
    this->hostname = host;
    this->inetAddress = InetAddress::getAnylocalAddress();
    bzero(&(this->addr6_), sizeof(this->addr6_));
  }
}

string InetSocketAddress::getHostName() {
  return this->hostname;
}

uint16_t InetSocketAddress::getPort() {
  return this->port;
}

InetAddress InetSocketAddress::getAddress() {
  return this->inetAddress;
}

uint16_t InetSocketAddress::getFamily() {
  if (Unresolved)
    return AF_INET;
  return this->inetAddress.getFamily();
}

string InetSocketAddress::toString() {
  stringstream ss;
  if (Unresolved) {
    ss << "[" << hostname << "]:" << port;
  } else {
    ss << "[" << inetAddress.toString() << "]:" << port;
  }
  return ss.str();
}

string InetSocketAddress::getHostAddress() {
  if (Unresolved)
    return "";
  else
    return inetAddress.getHostAddress();
}

bool InetSocketAddress::isUnresolved() {
  return this->Unresolved;
}

bool InetSocketAddress::isIPv6() {
  if (Unresolved) {
    return false;
  }
  return inetAddress.isIPV6();
}

const struct sockaddr *const InetSocketAddress::getSockAddrP() const {
  //return sockets::sockaddr_cast(&addr6_);
  return &(as_posix_sockaddr());
}

int InetSocketAddress::checkPort(int port) {
  if (port < 0 || port > 0xFFFF) {
    stringstream ss;
    ss << "port out of range:" << port;
    throw IllegalArgumentException(ss.str());
  }
  return port;
}

string InetSocketAddress::checkHost(const string &hostname) {
  if (hostname.length() == 0)
    throw IllegalArgumentException("hostname can't be null");
  return hostname;
}
