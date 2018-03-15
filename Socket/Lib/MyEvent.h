#pragma once
#define Lock_Succeed 1
#define Lock_Fail 0
#include <mutex>

int Mtx_Lock(std::mutex &mtx);
int Mtx_Unlock(std::mutex &mtx);
int Mtx_Init(std::mutex &mtx,bool Value);
int Mtx_Wait(std::mutex &mtx);