#include <iostream>
#include <string>
#include <thread>
#include <MSocket.h>
#include <mutex>
#include "MyEvent.h"
#include <mutex>
#include <Public.h>
using namespace std;

std::mutex mtx;
int counter = 0;

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
            mtx.lock();
            counter++;
            mtx.unlock();
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
        MSleep(5,"ms");
    }
}


int main()
{
    MSocket sock;
    SOCKET sHost = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);      //创建套接字
    sock.Init();
    sock.Connect(sHost, "192.168.1.2", 9000);




}