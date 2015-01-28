/*
  Copyright (c) 2014-2015 by NASa Qian <nasacj@nasacj.net>
  This file is part of the woodycxx library.

  This software is distributed under BSD 3-Clause License.
  The full license agreement can be found in the LICENSE file.

  This software is distributed without any warranty.
*/

#ifndef Woodycxx_Net_InetSocketAddress_H_
#define Woodycxx_Net_InetSocketAddress_H_

#include <string>

//using namespace std;

namespace woodycxx { namespace net {

class InetSocketAddress
{
private:
    std::string hostName;
    int port;

public:

    InetSocketAddress(){}

    InetSocketAddress( std::string host, int port  )
        : hostName(host), port(port)
    {
    }

    InetSocketAddress( const char* host, int port  )
        : hostName(host), port(port)
    {
    }

    std::string getHostName() { return this->hostName; }
    int getPort() { return this->port; }

    InetSocketAddress& operator=(const InetSocketAddress& other)
    {
        this->hostName = other.hostName;
        this->port = other.port;
        return *this;
    }

    friend bool operator==(const InetSocketAddress& p1, const InetSocketAddress& p2)
    {
        return ( (p1.hostName == p2.hostName) && (p1.port == p2.port) );
    }

    friend bool operator!=(const InetSocketAddress& p1, const InetSocketAddress& p2)
    {
        return ( (p1.hostName != p2.hostName) || (p1.port != p2.port) );
    }

};

}}

#endif
