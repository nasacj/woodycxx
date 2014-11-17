#include "sysipc_threads.h"
#include <iostream>

using namespace woodycxx::sysipc;
using namespace std;

class Test_Thread
{
private:
    Tthread<Test_Thread>* test_thread;
public:
    static void runner (Test_Thread* d)
    {
        for (int i = 0; i < 10; i++)
        {
            cout << "Thread Hello World!" << endl;
            //Sleep(1000);
        }
        
    }

    Test_Thread()
    {
        test_thread = Tthread<Test_Thread>::create("Test", runner, NULL, thread::DEFAULT_STACKSIZE);
        test_thread->start();
    }

    ~Test_Thread()
    {
        if (test_thread)
        {
            delete test_thread;
        }
    }
};


#if 1
int main()
{
    Test_Thread tst;
    //Sleep(1000);
    cin.get();
    return 0;
}

#endif
