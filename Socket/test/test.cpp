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
#include<random>
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
    std::default_random_engine engine(time(nullptr));
    std::uniform_int_distribution<> dis(1, 10);
    auto dice = std::bind(dis, engine);
    for (int n = 0; n < 10; n++)
        std::cout << dice() << " " << std::endl;

}

