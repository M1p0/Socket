#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include "MyEvent.h"
#include <MSocket.h>
using namespace std;



void Test()
{
    SOCKET sHost;
    MSocket sock;
    string ip = "192.168.1.2";
    int port = 9000;
    sHost = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sock.Init();
    while (1)
    {
        int retVal = sock.Connect(sHost, ip.c_str(), port);
        if (retVal == 0)
        {
            cout << "connected" << endl;
            break;
        }
    }

    while (1)
    {
        Packet Packet_Send;

        char buf[BUF_SIZE];
        memset(buf, 0, BUF_SIZE);
        memcpy(buf, "root", 4);
        memset(&Packet_Send, 0, BUF_SIZE + 4);
        Packet_Send.Length = BUF_SIZE;
        memcpy(Packet_Send.Data, buf, BUF_SIZE);
        sock.Send(sHost, (char*)&Packet_Send, BUF_SIZE + 4);
        Sleep(5);
    }
}


int main()
{
    thread Rec[1000];

    for (int i = 0; i < 1000; i++)
    {

        Rec[i] = thread(Test);
        Rec[i].detach();
    }

    //thread t1(Test);
    //thread t2(Test);


    getchar();
}