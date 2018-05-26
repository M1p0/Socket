#pragma once
#include <iostream>
#include <string>
#include <string.h>
#include <thread>
#include <chrono>
using namespace std;

#ifdef _WIN32
#include <windows.h>
#endif // _WIN32




template <class T>
T MIN(T num1, T num2)
{
    return num1 < num2 ? num1 : num2;
}

template <class T>
T MAX(T num1, T num2)
{
    return num1 > num2 ? num1 : num2;
}

template <class T>
int Match(T data, T array[], int length)   //string not included
{
    for (int i = 0; i < length; i++)
    {
        if (array[i] == data)
        {
            return 1;
        }
    }
    return 0;
};

#ifdef _WIN32
int Program_Mutex();  //return  (int)nRet
                      //nRet=0  create mutex succeed
                      //nRet=1  program is already running
                      //nRet=-1 create mutex failed
#endif // _WIN32



inline void MSleep(long long t,const char* unit)
{
    if (strcmp(unit, "s")==0)
    {
        std::this_thread::sleep_for(std::chrono::seconds(t));
    }
    else if (strcmp(unit, "ms")==0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(t));
    }
    else
        cout << "wrong unit" << endl;
}
