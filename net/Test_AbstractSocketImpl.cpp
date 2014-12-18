#include "AbstractSocketImpl.h"
#include "InetSocketAddress.h"

using namespace woodycxx::net;

int main()
{
    AbstractSocketImpl socketImpl;
    InetSocketAddress address("127.0.0.1", 80);
    socketImpl.connect(address, 80);
    socketImpl.getInetSoecktAddress();
    return 0;
}
