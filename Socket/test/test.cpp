#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <windows.h>
#include "MyEvent.h"
using namespace std;


int main()
{
    std::mutex mtx;
    Mtx_Lock(mtx);
    Mtx_Unlock(mtx);
}

