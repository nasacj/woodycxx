#include "scoped_ptr.h"

#include <iostream>

using namespace std;



class Test_scoped_ptr
{
public:
    Test_scoped_ptr()
    {
        cout << "Test_scoped_ptr() ==>" << endl;
    }

    ~Test_scoped_ptr()
    {
        cout << "~Test_scoped_ptr() |||" << endl;
    }
};

void test()
{
    typedef woodycxx::smart_prt::scoped_ptr<Test_scoped_ptr> Tester;
    Tester tester(new Test_scoped_ptr());
}



int main()
{
    test();
    cin.get();
    return 0;
}
