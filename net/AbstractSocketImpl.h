/*
 * AbstractSocketImpl.h
 *
 *  Created on: 2014-12-16
 *      Author: qianchj
 */

#ifndef ABSTRACTSOCKETIMPL_H_
#define ABSTRACTSOCKETIMPL_H_

#include "AbstractSocket.h"
#include "SocketInputStream.h"
#include "SocketOutputStream.h"
#include <smart_ptr/shared_ptr.h>

using namespace std;
using namespace woodycxx::io;

namespace woodycxx { namespace net {

class SocketInputStream;
class SocketOutputStream;
typedef woodycxx::smart_prt::shared_ptr<SocketInputStream> SocketInputStreamPtr;
typedef woodycxx::smart_prt::shared_ptr<SocketOutputStream> SocketOutputStreamPtr;

class AbstractSocketImpl : public AbstractSocket
{
private:
    string ip;

    enum CONNECTION_RESET_STATE {CONNECTION_NOT_RESET, CONNECTION_RESET_PENDING, CONNECTION_RESET};
    int resetState;
    int timeout;    // timeout in millisec

    bool shut_rd;
    bool shut_wr;
    bool closePending;
    bool closed;
    bool connected;
    
    SocketInputStreamPtr    inputStreamPtr;
    SocketOutputStreamPtr   outputStreamPtr;

public:
    AbstractSocketImpl() : shut_rd(false), shut_wr(false), closePending(false), connected(false) {}
    virtual ~AbstractSocketImpl() {}

    virtual int connect(string host, int port);

    virtual int connect(InetSocketAddress& address, int port);

    virtual void bind(InetSocketAddress& host, int port);

    virtual void listen(int backlog);

    virtual void accept(AbstractSocket& s);

    virtual InputStream& getInputStream();

    virtual OutputStream& getOutputStream();

    virtual int available();

    virtual void close();

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

}}//end of namspace woodycxx::net


#endif
