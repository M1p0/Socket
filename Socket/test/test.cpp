#include <iostream>
#include <string>
#include <thread>
#include <MSocket.h>
#include <mutex>
#include "MyEvent.h"
#include <mutex>
#include <Public.h>
#include <unordered_map>
#include <CFileIO.h>
using namespace std;

int funA(int a)
{
    cout << "funA:" << a << endl;
    return 0;
}


int funB(int a)
{
    cout << "funB:" << a << endl;
    return 0;
}


mutex mtx;
void Locker(string s)
{
    while (true)
    {
        //if (mtx.try_lock())
        //{
        //    cout << "Process " << s << endl;
        //    mtx.unlock();
        //}
        mtx.lock();
        cout << "Process " << s << endl;
        mtx.unlock();
    }
}

int main()
{
    thread A(Locker, "A");
    thread B(Locker, "B");
    A.join();
    B.join();
}