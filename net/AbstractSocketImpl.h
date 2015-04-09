/*
  Copyright (c) 2014-2015 by NASa Qian <nasacj@nasacj.net>
  This file is part of the woodycxx library.

  This software is distributed under BSD 3-Clause License.
  The full license agreement can be found in the LICENSE file.

  This software is distributed without any warranty.
*/

#ifndef ABSTRACTSOCKETIMPL_H_
#define ABSTRACTSOCKETIMPL_H_

#include "AbstractSocket.h"
#include <io/FileDescriptor.h>
#include "SocketInputStream.h"
#include "SocketOutputStream.h"
//#include <smart_ptr/shared_ptr.h>
//#include <boost/shared_ptr.hpp>
//#include <boost/enable_shared_from_this.hpp>
#include <memory>

using namespace std;
using namespace woodycxx::io;

namespace woodycxx { namespace net {

class SocketInputStream;
class SocketOutputStream;
//typedef woodycxx::smart_prt::shared_ptr<SocketInputStream> SocketInputStreamPtr;
//typedef woodycxx::smart_prt::shared_ptr<SocketOutputStream> SocketOutputStreamPtr;
typedef shared_ptr<SocketInputStream> SocketInputStreamPtr;
typedef shared_ptr<SocketOutputStream> SocketOutputStreamPtr;

class AbstractSocketImpl : public AbstractSocket
{
private:

    enum CONNECTION_RESET_STATE {CONNECTION_NOT_RESET, CONNECTION_RESET_PENDING, CONNECTION_RESET};
    int resetState;
    int timeout;    // timeout in millisec

    bool shut_rd;
    bool shut_wr;
    bool closePending;
    bool closed;
    bool connected;
    
    //SocketInputStreamPtr    inputStreamPtr;
    //SocketOutputStreamPtr   outputStreamPtr;
    InputStreamPtr    inputStreamPtr;
    OutputStreamPtr   outputStreamPtr;

    InetAddress address;
    FileDescriptor fileHandler;

public:
    AbstractSocketImpl( const InetAddress& addr) : shut_rd(false), shut_wr(false), closePending(false), connected(false), address(addr) {}
    AbstractSocketImpl( const string& ip, int port): shut_rd(false), shut_wr(false), closePending(false), connected(false), address(ip, port) {}
    virtual ~AbstractSocketImpl() {}

    std::shared_ptr<AbstractSocketImpl> shared_from_this()
    {
        return std::static_pointer_cast<AbstractSocketImpl>(AbstractSocket::shared_from_this());
    }

    virtual int connect(const string& host, int port);

    virtual int connect(const InetAddress& address);

    virtual int connect();

    virtual void bind(const InetAddress& host);

    virtual void listen(int backlog);

    virtual void accept(const AbstractSocket& s);

    virtual InputStreamPtr getInputStream();

    virtual OutputStreamPtr getOutputStream();

    virtual int available();

    virtual void close();

    InetAddress& getInetSoecktAddress() { return this->address; }

    FileDescriptor& getFileDescriptor() { return this->fileHandler; }

    string getIpString();

    uint16_t getPortString();

    string getIpPortString();

    virtual bool isClosed()
    {
        return closed;
    }

    bool isConnectionReset()
    {
        //TODO synchronized
        return (resetState == CONNECTION_RESET);
    }

    bool isConnectionResetPending()
    {
        return (resetState == CONNECTION_RESET_PENDING);
    }

    void setConnectionReset()
    {
        resetState = CONNECTION_RESET;

    }

    void setConnectionResetPending()
    {
        if (resetState == CONNECTION_NOT_RESET)
            resetState = CONNECTION_RESET_PENDING;

    }

    int getTimeout()
    {
        return timeout;
    }

    bool isClosedOrPending()
    {
        /*
         * Lock on fdLock to ensure that we wait if a
         * close is in progress.
         */
        if (closePending)
        //if (closePending || (fd == null))
        {
            return true;
        } 
        else
        {
            return false;
        }
    }


};

typedef shared_ptr<AbstractSocketImpl> AbstractSocketImplPtr;

}}//end of namspace woodycxx::net


#endif
