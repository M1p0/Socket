#include "MyEvent.h"
//#include <unistd.h>
#include<windows.h>
#include <mutex>


int Mtx_Lock(std::mutex &mtx)
{
    mtx.lock();
    return 0;
}

int Mtx_Unlock(std::mutex &mtx)
{
    mtx.unlock();
    return 0;
}

int Mtx_Init(std::mutex &mtx,bool Value)
{
    if (Value==TRUE)
        mtx.lock();
    else
        mtx.unlock();
    return 0;
}

int Mtx_Wait(std::mutex &mtx)
{
    while (mtx.try_lock() != 1)
    {
        Sleep(1);
    }
    mtx.unlock();
    return 0;
}