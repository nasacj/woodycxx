/*
 * InetSocketAddress.h
 *
 *  Created on: 2014-7-18
 *      Author: qianchj
 */

#ifndef InetSocketAddress_H_
#define InetSocketAddress_H_

#include <string>

//using namespace std;

namespace woodycxx { namespace net {

class InetSocketAddress
{
private:
    std::string hostName;
    int port;

public:
    InetSocketAddress( std::string host, int port  )
        : hostName(host), port(port)
    {
    }

    InetSocketAddress( char* host, int port  )
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
