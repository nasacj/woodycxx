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


void test_array()
{
    typedef woodycxx::smart_prt::scoped_array<Test_scoped_array> Testers;
    Testers testers(new Test_scoped_array[10]);
    //testers[-1];
}

int main()
{
    test_array();
    cin.get();
    return 0;
}


