#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include "MyEvent.h"
using namespace std;


int main()
{
    string a = "hello";
    char s[1024];
    memset(s, 0, 1024);
    memcpy(s, a.c_str(), a.size());


    cout << s << endl;
}