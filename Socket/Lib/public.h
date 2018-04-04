#pragma once
#include <iostream>
#include <string>
#include <windows.h>

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


int Program_Mutex();  //return  (int)nRet
                      //nRet=0  create mutex succeed
                      //nRet=1  program is already running
                      //nRet=-1 create mutex failed

