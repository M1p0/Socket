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

struct stu
{
    int array[];

};


int main()
{
    int i=0, a[10];
    char c;
    while (true)
    {
        scanf_s("%d%c", a + i, &c);
        printf_s("%d\n", a[i]);
        if (c == '\n') break;
    }

}

