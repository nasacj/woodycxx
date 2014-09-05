/*
 * InetSocketAddress.h
 *
 *  Created on: 2014-7-18
 *      Author: qianchj
 */

#ifndef InetSocketAddress_H_
#define InetSocketAddress_H_

#include <string>

using namespace std;

namespace woodycxx { namespace net {

class InetSocketAddress
{
private:
    string hostName;
    int port;

public:
    explicit InetSocketAddress( string host, int port  )
        : hostName(host), port(port)
    {
    }

    string getHostName() { return this->hostName; }
    int getPort() { return this->port; }

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