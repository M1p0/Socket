#include <iostream>
#include <string>
#include <thread>
#include <mutex
#include "MyEvent.h"
using namespace std;

#pragma pack(1)
struct Data
{
    int length = 0;
    char buf[1024];
};
#pragma pack()

int main()
{
    char *s = new char[512];
    memset(s, 0, 512);
    cin >> s;
    cout << s[5] << endl;
}