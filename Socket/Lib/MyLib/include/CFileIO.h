#pragma once
#include <iostream>
#include <cstdio>
#include <string.h>
#include <stdint.h>
using namespace std;

#ifdef _WIN32
#include "windows.h"
#endif // _WIN32



class CFileIO
{
public:
    CFileIO();
    ~CFileIO();
    int Init();
    void GetSize(const char* szPath, int64_t* Size);
    int Read(const char* szPath, char* Buffer, long Offset, int64_t Buffer_Size);
    void Write(const char* szPath, const char* szData, long Offset, int64_t Buffer_Size);
    void Copy(const char* SourceFile, const char* NewFile);
    void SetBuffSize(int64_t Size);
private:
    int64_t Shared_Buff_Size = 1024 * 1024;  //ª∫¥Ê¥Û–°1MB
    char* Shared_Buffer = nullptr;
};