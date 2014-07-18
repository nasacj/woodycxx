/*
 * InetAddress.h
 *
 *  Created on: 2014-7-18
 *      Author: qianchj
 */

#ifndef INETADDRESS_H_
#define INETADDRESS_H_

#include <string>

namespace woodycxx { namespace net {

class InetAddress
{
private:
    string hostName;
    int address;
    int family;

public:
    InetAddressHolder();

    InetAddressHolder(String hstN, int addr, int fam);


};

}}

#endif