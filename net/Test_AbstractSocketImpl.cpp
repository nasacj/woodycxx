#include "AbstractSocketImpl.h"
#include "InetSocketAddress.h"
#include <iostream>

using namespace woodycxx::net;
using namespace std;

int main()
{
#define MAXLINE 10
    char    recvline[MAXLINE + 1];
    int n;

    AbstractSocketImpl socketImpl;
    InetSocketAddress address("127.0.0.1", 12345);
    int error_no = socketImpl.connect(address, 12345);
    if (error_no != 0)
    {
        cout << "connect failed: error_no=" << hex << error_no << endl;
        return -1;
    }
    InputStream& inputstream = socketImpl.getInputStream();
    OutputStream& outputstream = socketImpl.getOutputStream();
    while ( (n = inputstream.read(recvline, MAXLINE)) > 0 )
    {
        recvline[n] = 0;
        cout << recvline;
        int n_written = outputstream.write(recvline, n);
        if (n_written <= 0 )
        {
            cout << "Write Error!" << endl;
            continue;
        }
    }
    if (n != 0)
    {
        cout << endl << "read error: error_no=" << n << endl;
        return -1;
    }
    return 0;
}
