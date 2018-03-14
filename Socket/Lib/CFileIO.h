#pragma once
#include <iostream>
#include <cstdio>
#include <string>
using namespace std;

class CFileIO
{
public:
    fpos_t FileSize;
    int buff_size=1024*1024;  //ª∫¥Ê¥Û–°1MB
    void GetSize(char* szPath);
    char* Read(char* szPath, long offset, long size);
    void  Write(char* szPath, char* szData, long offset, long Size);
    void Copy(char* SourceFile, char* NewFile);
};

