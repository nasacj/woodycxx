/*
 * Socket.h
 *
 *  Created on: 2014-7-14
 *      Author: qianchj
 */

#ifndef SOCKET_H_
#define SOCKET_H_

#include <string.h>

namespace woodycxx {

class Socket
{
public:
	Socket(string ipAdd);
	virtual ~Socket(){}

};

}


#endif /* SOCKET_H_ */