#pragma once
//需要自己添加mysql头文件目录
#include <iostream>
#include <mysql.h>
#include <string>
#include <vector>
using namespace std;
class MDatabase
{
public:
    MDatabase();
    ~MDatabase();
    int Connect(const char* IP,const char* User,const char* Password,const char* DbName,int Port);
    int SetEncoding(const char* Encoding);
    int ExecSQL(const char* SQL, vector<vector<string>> &Result, int &nRow);
    void Init();
private:
    MYSQL * pConn;
};
