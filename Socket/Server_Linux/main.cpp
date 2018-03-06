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


const int BUF_SIZE = 512;
struct Cli_Info
{
    string ip = "0.0.0.0";
    int port = 0;
};

#pragma pack(1)
struct SPacket
{
    int Length;
    char Data[BUF_SIZE];
};
#pragma pack()

std::mutex mtx_CIP;
std::mutex mtx_CSocket;
queue <string> MsgQueue;  //消息队列
vector <Cli_Info> CIP;    //客户端IP
int retVal;         //返回值
vector <SOCKET> CSocket;    //客户端套接字
std::mutex  Stop;
std::mutex Ender;
SOCKET sServer;        //服务器套接字  
SOCKET sClient;        //客户端套接字  

int Certificate(SOCKET Client)
{
    int Length;
    recv(Client, &Length, 4, MSG_NOSIGNAL);
    char* Passwd = new char[Length];
    memset(Passwd, 0, BUF_SIZE + 4);
    recv(Client, Passwd, Length, MSG_NOSIGNAL);

    if (strcmp(Passwd,"root")==0)
    {
        delete[] Passwd;
        return 0;
    }
    else
    {
        delete[] Passwd;
        return -1;
    }
}

int Forward()
{
    string Msg;
    while (true)
    {
        Mtx_Lock(mtx_CSocket);
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
    sockaddr_in peeraddr;  //区分in_addr
    socklen_t len = sizeof(peeraddr);
    std::mutex Locker;
    Mtx_Lock(Locker);
    SOCKET Client = sClient;
    Mtx_Unlock(Locker);
    
    if (Certificate(Client)!=0)
    {
        for (unsigned int i = 0; i < CSocket.size(); i++)
        {
            if (CSocket.at(i) == Client)
            {
                Mtx_Lock(mtx_CSocket);
                CSocket.erase(CSocket.begin() + i);
                Mtx_Unlock(mtx_CSocket);
            }
        }
        close(Client);
        return -1;
    }

    if (!getpeername(Client, (struct sockaddr *)&peeraddr, &len))
    {
        char peerip[18];
        inet_ntop(AF_INET, &peeraddr.sin_addr, peerip, sizeof(peerip));
        CInfo.ip = peerip;
        CInfo.port = ntohs(peeraddr.sin_port);  //IPV6需要使用inet_pton()

        Mtx_Lock(mtx_CIP);
        CIP.push_back(CInfo);
        Mtx_Unlock(mtx_CIP);
    }

    while (true)
    {
        if (Ender.try_lock() == 1)
        {
            cout << "Receiver stopped" << endl;
            return 0;
        }

        int Length;
        char* Data;
        retVal = recv(Client, &Length, 4, MSG_NOSIGNAL);

        Data = new char[Length];
        memset(Data, 0, BUF_SIZE + 4);

        //cout << "Length:" << Length << endl;
        retVal = recv(Client, Data, Length, MSG_NOSIGNAL);
        //cout << "retVal = " << retVal << endl;

        //for (auto i = 0; i < retVal; i++)
        //{
        //    printf("%02X", Data[i]);
        //}

        //if (retVal == -1)
        if (retVal <= 0)
        {
            cout << "recv failed!" << endl;
            for (unsigned int i = 0; i < CSocket.size(); i++)
            {
                if (CSocket.at(i) == Client)
                {
                    Mtx_Lock(mtx_CSocket);
                    CSocket.erase(CSocket.begin() + i);
                    Mtx_Unlock(mtx_CSocket);

                    Mtx_Lock(mtx_CIP);
                    CIP.erase(CIP.begin() + i);
                    Mtx_Unlock(mtx_CIP);
                }
            }
            close(Client);
            return -1;
        }
        cout << "receive: " << Data << endl;
        Msg = Data;
        delete[] Data;
        MsgQueue.push(Msg);


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
    Mtx_Init(Stop, true);
    Mtx_Init(Ender, true);
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
        if (cmd == "pause")
            Mtx_Unlock(Ender);
        else if (cmd == "continue")
            Mtx_Lock(Ender);
        sleep(1);
    }

    Mtx_Wait(Stop);
    return 0;
}

