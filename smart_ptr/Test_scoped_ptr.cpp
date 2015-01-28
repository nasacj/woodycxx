/*
  Copyright (c) 2014-2015 by NASa Qian <nasacj@nasacj.net>
  This file is part of the woodycxx library.

  This software is distributed under BSD 3-Clause License.
  The full license agreement can be found in the LICENSE file.

  This software is distributed without any warranty.
*/

#include "scoped_ptr.h"
#include <base/Types.h>
#include <iostream>

using namespace std;

class Test_scoped_ptr;
typedef woodycxx::smart_prt::scoped_ptr<Test_scoped_ptr> Tester;

class Test_scoped_ptr
{
public:
    Test_scoped_ptr()
    {
        cout << "Test_scoped_ptr() ==>" << endl;
    }

    void inline inline_fun()
    {
        cout << "Start inline_fun()..." << endl;
        Tester tester(new Test_scoped_ptr());
        cout << "End inline_fun()..." << endl;
    }

    ~Test_scoped_ptr()
    {
        cout << "~Test_scoped_ptr() |||" << endl;
    }
};

void test_str()
{
    //typedef woodycxx::smart_prt::scoped_ptr<Test_scoped_ptr> Tester;
    Tester tester(new Test_scoped_ptr());
    tester->inline_fun();

    {
        Tester tester(new Test_scoped_ptr());
    }

    typedef woodycxx::smart_prt::scoped_ptr<uint8> Byte;

    uint8* byte_b = Byte(new uint8).get();
    Byte a_byte(new uint8);
    cout << "AFTER ---> uint8* byte_b = Byte(new uint8).get()" << endl;
}

int main()
{
    test_str();
    return 0;
}
