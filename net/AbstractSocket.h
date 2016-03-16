/*
  Copyright (c) 2014-2015 by NASa Qian <nasacj@nasacj.net>
  This file is part of the woodycxx library.

  This software is distributed under BSD 3-Clause License.
  The full license agreement can be found in the LICENSE file.

  This software is distributed without any warranty.
*/

#ifndef ABSTRACTSOCKET_H_
#define ABSTRACTSOCKET_H_

#include <io/InputStream.h>
#include <io/OutputStream.h>
#include "InetSocketAddress.h"
//#include <base/noncopyable.h>
#include <boost/noncopyable.hpp>
#include <string>

using namespace std;
using namespace woodycxx::io;

namespace woodycxx { namespace net {



class AbstractSocket : boost::noncopyable, public enable_shared_from_this<AbstractSocket>
{
//protected:
public:

    AbstractSocket() {}

    /**
     * Connects this socket to the specified port on the named host.
     *
     * @param      host   the name of the remote host.
     * @param      port   the port number.
     *
     */
    virtual int connect(const string& host, int port) = 0;

    /**
     * Connects this socket to the specified port number on the specified host.
     *
     * @param      address   InetAddress.
     *
     */
    virtual int connect(const InetSocketAddress& address) = 0;

    /**
     * Connects this socket to the specified port number on the initialized host.
     *
     * @param      address   InetAddress.
     *
     */
    virtual int connect() = 0;

    /**
     * Binds this socket to the specified local IP address and port number.
     *
     * @param      host   an IP address that belongs to a local interface.
     * @param      port   the port number.
     */
    virtual void bind(const InetSocketAddress& host) = 0;

    /**
     * Sets the maximum queue length for incoming connection indications
     * (a request to connect) to the <code>count</code> argument. If a
     * connection indication arrives when the queue is full, the
     * connection is refused.
     *
     * @param      backlog   the maximum length of the queue.
     *
     */
    virtual void listen(int backlog) = 0;

    /**
     * Accepts a connection.
     *
     * @param      s   the accepted connection.
     *
     */
    virtual void accept(const AbstractSocket& s) = 0;

    /**
     * Returns an input stream for this socket.
     *
     * @return     a stream for reading from this socket.
     *
    */
    virtual InputStreamPtr getInputStream() = 0;


    /**
     * Returns an output stream for this socket.
     *
     * @return     an output stream for writing to this socket.
     * @exception  IOException  if an I/O error occurs when creating the
     *               output stream.
     */
    virtual OutputStreamPtr getOutputStream() = 0;

    /**
     * Returns the number of bytes that can be read from this socket
     * without blocking.
     *
     * @return     the number of bytes that can be read from this socket
     *             without blocking.
     * @exception  IOException  if an I/O error occurs when determining the
     *               number of bytes available.
     */
    virtual int available()  = 0;

    /**
     * Closes this socket.
     *
     * @exception  IOException  if an I/O error occurs when closing this socket.
     */
    virtual void close() = 0;

	virtual void closeRead() = 0;

	virtual void closeWrite() = 0;

    virtual ~AbstractSocket() {}

};

}}

#endif /* ABSTRACTSOCKET_H_ */
