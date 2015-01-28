/*
  Copyright (c) 2014-2015 by NASa Qian <nasacj@nasacj.net>
  This file is part of the woodycxx library.

  This software is distributed under BSD 3-Clause License.
  The full license agreement can be found in the LICENSE file.

  This software is distributed without any warranty.
*/

#include "scoped_array.h"
#include <iostream>

using namespace std;

class Test_scoped_array
{
public:
    Test_scoped_array()
    {
        cout << "Test_scoped_array() ==>" << endl;
    }

    ~Test_scoped_array()
    {
        cout << "~Test_scoped_array() |||" << endl;
    }
};

typedef woodycxx::smart_prt::scoped_array<Test_scoped_array> Testers;

class Test_scoped_array_wrap
{
public:
    Test_scoped_array_wrap(int size)
    {
        cout << "Test_scoped_array_wrap() ==>" << endl;
        testers.reset(new Test_scoped_array[size]);
    }

    ~Test_scoped_array_wrap()
    {
        cout << "~Test_scoped_array_wrap() |||" << endl;
    }

private:
    Testers testers;
};


void test_array()
{
    cout << "--- test_array --- " << endl;
    Testers testers(new Test_scoped_array[10]);
    //testers[-1];
    cout << "--- END test_array --- " << endl;
}

void test_array_wrap()
{
    cout << "--- test_array_wrap --- " << endl;
    Test_scoped_array_wrap array_wrap_obj(5);
    cout << "--- END test_array_wrap --- " << endl;
}


int main()
{
    test_array();
    test_array_wrap();
    cout << "--- END main --- " << endl;
    cin.get();
    return 0;
}


