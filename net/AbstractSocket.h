/*
 * AbstractSocket.h
 *
 *  Created on: 2014-7-18
 *      Author: qianchj
 */

#ifndef ABSTRACTSOCKET_H_
#define ABSTRACTSOCKET_H_

#include <io/InputStream.h>
#include <io/OutputStream.h>
#include <io/FileDescriptor.h>
#include "InetSocketAddress.h"
#include <string>

using namespace std;
using namespace woodycxx::io;

namespace woodycxx { namespace net {



class AbstractSocket
{
//protected:
public:

    AbstractSocket() {}

    AbstractSocket(string ip, int port) : address(ip, port) {}

    AbstractSocket(InetSocketAddress in_address) : address(in_address) {}

    /**
     * Connects this socket to the specified port on the named host.
     *
     * @param      host   the name of the remote host.
     * @param      port   the port number.
     *
     */
    virtual int connect(string host, int port) = 0;

    /**
     * Connects this socket to the specified port number on the specified host.
     *
     * @param      address   the IP address of the remote host.
     * @param      port      the port number.
     *
     */
    virtual int connect(InetSocketAddress& address, int port) = 0;

    /**
     * Binds this socket to the specified local IP address and port number.
     *
     * @param      host   an IP address that belongs to a local interface.
     * @param      port   the port number.
     */
    virtual void bind(InetSocketAddress& host, int port) = 0;

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
    virtual void accept(AbstractSocket& s) = 0;

    /**
     * Returns an input stream for this socket.
     *
     * @return     a stream for reading from this socket.
     *
    */
    virtual InputStream& getInputStream() = 0;


    /**
     * Returns an output stream for this socket.
     *
     * @return     an output stream for writing to this socket.
     * @exception  IOException  if an I/O error occurs when creating the
     *               output stream.
     */
    virtual OutputStream& getOutputStream() = 0;

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

    InetSocketAddress& getInetSoecktAddress() { return this->address; }

    FileDescriptor& getFileDescriptor() { return this->fileHandler; }

    int getPort() { return this->port; }

    virtual ~AbstractSocket() {}

protected:

    InetSocketAddress address;
    int port;
    FileDescriptor fileHandler;

};

}}

#endif /* ABSTRACTSOCKET_H_ */
