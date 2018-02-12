#include <netinet/in.h> //套接字地址结构
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <thread>
#include <string>
#include <string.h>
#include <queue>
#include <vector>
#include <mutex>
#include "MyEvent.h"
#define SOCKET int
using namespace std;


struct Cli_Info
{
    string ip = "0.0.0.0";
    int port = 0;
};

std::mutex mtx_CIP;
std::mutex mtx_CSocket;
const int BUF_SIZE = 64;
queue <string> MsgQueue;  //消息队列
vector <Cli_Info> CIP;    //客户端IP
int retVal;         //返回值

vector <SOCKET> CSocket;    //客户端套接字
std::mutex  Stop;
std::mutex Ender;
SOCKET sServer;        //服务器套接字  
SOCKET sClient;        //客户端套接字  

int Forward()
{
    string Msg;
    while (true)
    {
        mtx_CSocket.lock();
        if (CSocket.size() != 0 && MsgQueue.size() != 0)
        {
            vector<SOCKET>::iterator it;
            for (it = CSocket.begin(); it != CSocket.end(); it++)
            {
                Msg = MsgQueue.front().c_str();
                send(*it, Msg.c_str(), Msg.size(), MSG_NOSIGNAL);
            }
            MsgQueue.pop();
        }
        mtx_CSocket.unlock();
        sleep(1);
    }
    return 0;
}


int Receiver()
{
    string Msg;
    Cli_Info CInfo;
    char buf[BUF_SIZE];  //接收客户端数据 
    sockaddr_in peeraddr;  //区分in_addr
    socklen_t len = sizeof(peeraddr);
    std::mutex Locker;
    Locker.lock();
    SOCKET Client = sClient;
    Locker.unlock();

    if (!getpeername(Client, (struct sockaddr *)&peeraddr, &len))
    {
        char peerip[18];
        inet_ntop(AF_INET, &peeraddr.sin_addr, peerip, sizeof(peerip));
        CInfo.ip = peerip;
        CInfo.port = ntohs(peeraddr.sin_port);  //IPV6需要使用inet_pton()

        mtx_CIP.lock();
        CIP.push_back(CInfo);
        mtx_CIP.unlock();
    }

    while (true)
    {/////////////////////////////////////////////////////////
        if (Ender.try_lock()==1)
        {
            cout << "Receiver stopped" << endl;
            return 0;
        }

        memset(buf, 0, BUF_SIZE);
        retVal = recv(Client, buf, BUF_SIZE, MSG_NOSIGNAL);
        if (retVal == -1)
        {
            cout << "recv failed!" << endl;

            for (unsigned int i = 0; i < CSocket.size(); i++)
            {
                if (CSocket.at(i) == Client)
                {
                    mtx_CSocket.lock();
                    CSocket.erase(CSocket.begin() + i);
                    mtx_CSocket.unlock();

                    mtx_CIP.lock();
                    CIP.erase(CIP.begin() + i);
                    mtx_CIP.unlock();
                }
            }

            return -1;
        }
        if (buf[0] == '0')
            break;
        Msg = buf;
        cout << "receive: " << Msg << endl;
        MsgQueue.push(Msg);


        if (Msg == "root@admin")
        {
            close(sServer);   //关闭套接字  
            Mtx_Unlock(Stop);
            return 0;
        }
        if (Msg == "show")
        {
            vector<Cli_Info>::iterator it;
            for (it = CIP.begin(); it != CIP.end(); it++)
            {
                cout << it->ip << ":" << it->port << endl;
            }
        }
        sleep(1);
    }
    return 0;
}

int GenRec()
{

    sockaddr_in addrClient;
    socklen_t addrClientlen = sizeof(addrClient);

    while (true)
    {
        sClient = accept(sServer, (sockaddr*)&addrClient, &addrClientlen);
        if (sClient == -1)
        {
            cout << "Accept failed!" << endl;
            return -1;
        }
        else
        {
            cout << "connected" << endl;
            Mtx_Lock(mtx_CSocket);
            CSocket.push_back(sClient);
            Mtx_Unlock(mtx_CSocket);
            thread Rec(Receiver);
            Rec.detach();
        }
        sleep(1);
    }
}


int main()
{
    Mtx_Init(Stop,true);
    Mtx_Init(Ender,true);
    sockaddr_in addrServ;      //服务器地址  
    sServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (sServer == -1)
    {
        cout << "Socket failed!" << endl;
        close(sServer);
        return -1;
    }
    addrServ.sin_family = AF_INET;
    addrServ.sin_port = htons(9000);
    addrServ.sin_addr.s_addr = INADDR_ANY;
    retVal = ::bind(sServer, (sockaddr*)&addrServ, sizeof(addrServ));
    if (retVal == -1)
    {
        cout << "bind failed!" << endl;
        close(sServer);   //关闭套接字    
        retVal = 0;
        return -1;
    }
    retVal = listen(sServer, 5);
    if (retVal == -1)
    {
        cout << "listen failed!" << endl;
        close(sServer);   //关闭套接字  
        retVal = 0;
        return -1;
    }
    thread Gen(GenRec);
    Gen.detach();
    thread Fwd(Forward);
    Fwd.detach();
    string cmd;
    while (cin >> cmd)
    {
        if (cmd == "stop")
            Mtx_Unlock(Ender);
        else if (cmd == "start")
            Mtx_Lock(Ender);
        sleep(1);
    }

    Mtx_Wait(Stop);
    return 0;
}

