/*
  Copyright (c) 2014-2015 by NASa Qian <nasacj@nasacj.net>
  This file is part of the woodycxx library.

  This software is distributed under BSD 3-Clause License.
  The full license agreement can be found in the LICENSE file.

  This software is distributed without any warranty.
*/

#include "AbstractSocketImpl.h"
#include "InetSocketAddress.h"
#include <iostream>
#ifndef WIN32
#include <signal.h>
#endif

using namespace woodycxx::net;
using namespace std;

int Test_AbstractSocketImpl() {
#define MAXLINE 10
  char recvline[MAXLINE + 1];
  int n;

  InetSocketAddress address("127.0.0.1", 12345);
  auto socketImpl = AbstractSocketImpl::createSocket(address);

  try {

    cout << "Connect to " << address.toString() << endl;
    int error_no = socketImpl->connect();
    if (error_no != 0) {
      cout << "connect failed: error_no=" << hex << error_no << endl;
      return -1;
    }
    cout << "Connect successfully..." << endl;
    auto inputstream = socketImpl->getInputStream();
    auto outputstream = socketImpl->getOutputStream();
    InputStreamPtr inputstream2 = socketImpl->getInputStream();
    while ((n = inputstream->read(recvline, MAXLINE)) > 0) {
      recvline[n] = 0;
      cout << string(recvline);
      int n_written = outputstream->write(recvline, n);
      if (n_written <= 0) {
        cout << endl << "Write Error! error_no=" << hex << n_written << endl;
        continue;
      }
    }
    if (n != 0) {
      cout << endl << "read error: error_no=" << hex << n << endl;
      return -1;
    }

  }
  catch (Exception &e) {
    Finally aa([&] {
      socketImpl->close();
      cout << "This is Finally!" << endl;
    });
    cerr << e.what() << endl;
    return -1;
  }

  return 0;
}

int main() {
#ifdef __linux__
  signal(SIGPIPE, SIG_IGN);
#endif
#if defined(__APPLE__)
  cout << "<<< This is Mac System >>>" << endl;
#endif
#if defined(_WINDOWS_)
  cout << "<<< This is Windows System >>>" << endl;
#endif
#if defined(__linux__)
  cout << "<<< This is Linux System >>>" << endl;
#endif
#if defined(__CYGWIN__)
  cout << "<<< This is Cygwin System >>>" << endl;
#endif

  cout << "Test_AbstractSocketImpl Start..." << endl;
  int ret = Test_AbstractSocketImpl();
  cout << "Test_AbstractSocketImpl End ===> ret = " << hex << ret << endl;
  return ret;
}
