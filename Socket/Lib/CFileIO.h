#pragma once
#include <iostream>
#include <cstdio>
using namespace std;

class CFileIO
{
public:
    CFileIO();
    ~CFileIO();
    fpos_t FileSize;
    void GetSize(char* szPath);
    char* Read(char* szPath, long offset, long size);
    void  Write(const char* szPath, const char* szData, long offset, long Size);
    void Copy(char* SourceFile, char* NewFile);
private:
    fpos_t buff_size = 1024 * 1024;  //ª∫¥Ê¥Û–°1MB
    char* buffer;
};

