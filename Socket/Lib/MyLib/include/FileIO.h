#pragma once
#include <fstream>
#include <iostream>
#include <string>
#include <string.h>
using namespace std;

#ifdef _WIN32
#include <windows.h>
#endif // _WIN32

class FileIO
{
public:
    FileIO()
    {
        std::ios::sync_with_stdio(false);   //关闭与stdio的同步 加快I/O速度
    }
    void Create(string location);//例如File.Create("c:\\test.txt")
    void Write(string location, string data);
    void Read(const char* location, char* buffer, int mode);  //0为文本方式 1为二进制方式 
    void Copy(string SourceFile, string NewFile);
    void ListDir(const char* lpFileName);
    void Backup();
    void CopyFolder(string szPath, string szTarget);
};
