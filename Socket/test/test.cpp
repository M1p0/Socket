#include <iostream>
#include <string>
#include <thread>
#include <MSocket.h>
#include <mutex>
#include "MyEvent.h"
#include <mutex>
#include <Public.h>
#include <unordered_map>
#include <CFileIO.h>
using namespace std;

int funA(int a)
{
    cout << "funA:" << a << endl;
    return 0;
}


int funB(int a)
{
    cout << "funB:" << a << endl;
    return 0;
}


int main()
{
    //const char* command = "funA";
    //int(*pA)(int) = &funA;
    //int(*pB)(int) = &funB;
    //unordered_map<const char*,int(*)(int)> map;
    //map.insert(pair<const char*, int(*)(int)>("funA",pA));
    //map.insert(pair<const char*, int(*)(int)>("funB", pB));
    //unordered_map<const char*, int(*)(int)>::iterator it;
    //it = map.find(command);
    //it->second(6);
    //string id = "143";
    //string password = "123";

    //string SQL = R"(select ID,username from user where ID=")" + id + R"(")" + R"(and password=")" + password + R"(";)";
    //cout << SQL << endl;
}