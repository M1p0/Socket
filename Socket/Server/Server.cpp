#undef  WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include <thread>
#include <string>
#include <queue>
#include <vector>
#include <mutex>
#include <MSocket.h>
#include <windows.h>
#include <MyEvent.h>
#pragma comment(lib,"Lib.lib")
using namespace std;


std::mutex mtx_CIP;
std::mutex mtx_CSocket;
std::mutex mtx_Client;
std::mutex mtx_MsgQue;
std::mutex mtx_Packet;
queue <Packet> Packet_Receive;
vector <Cli_Info> CIP;    //客户端IP
vector <SOCKET> CSocket;    //客户端套接字
std::mutex Stop;
std::mutex Ender;
SOCKET sServer;        //服务器套接字
SOCKET sClient;        //客户端套接字
MSocket sock;


int Certificate(SOCKET Client)
{
    int Length = 0;
    sock.Recv(Client, (char*)&Length, 4);
    char Passwd[1024];
    memset(Passwd, 0, 1024);
    sock.Recv(Client, Passwd, Length);

    if (strcmp(Passwd, "root") == 0)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}



int Forward()
{
    while (true)
    {
        Mtx_Lock(mtx_CSocket);
        Mtx_Lock(mtx_Packet);
        if (CSocket.size() != 0 && Packet_Receive.size() != 0)
        {
            vector<SOCKET>::iterator it;
            for (it = CSocket.begin(); it != CSocket.end(); it++)
            {
                Packet Packet_Send;
                Packet_Send = Packet_Receive.front();
                int retVal = sock.Send(*it, (char*)&Packet_Send, BUF_SIZE + 4);
                if (retVal == -1)
                {
                    cout << "Forward Failed" << endl;
                }
            }
            Packet_Receive.pop();
        }
        Mtx_Unlock(mtx_Packet);
        Mtx_Unlock(mtx_CSocket);
        Sleep(1);
    }
    return 0;
}


int Receiver()
{
    Cli_Info CInfo;
    Mtx_Lock(mtx_Client);
    SOCKET Client = sClient;
    Mtx_Unlock(mtx_Client);
    if (Certificate(Client) != 0)
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
        closesocket(Client);
        return -1;
    }

    if (sock.Getpeername(Client, CInfo) == 0)
    {
        Mtx_Lock(mtx_CIP);
        CIP.push_back(CInfo);
        Mtx_Unlock(mtx_CIP);
    }
    else
    {
        cout << "Getpeername failed" << endl;
        return -1;
    }

    while (true)
    {
        if (Ender.try_lock() == 1)
        {
            cout << "Receiver stopped" << endl;
            return 0;
        }
        int retVal = 0;
        char buf[BUF_SIZE];  //接收客户端数据 
        memset(buf, 0, BUF_SIZE);
        int Length = 0;
        retVal = sock.Recv(Client, (char*)&Length, 4);
        retVal = sock.Recv(Client, buf, Length);
        if (retVal <= 0)
        {
            cout << "recv failed!" << endl;
            Mtx_Lock(mtx_CSocket);
            Mtx_Lock(mtx_CIP);
            for (unsigned int i = 0; i < CSocket.size(); i++)
            {
                if (CSocket.at(i) == Client)
                {
                    CSocket.erase(CSocket.begin() + i);
                    CIP.erase(CIP.begin() + i);
                }
            }
            Mtx_Unlock(mtx_CSocket);
            Mtx_Unlock(mtx_CIP);
            return -1;
        }
        if (buf[0] == '0')
            break;
        Packet PRecv;
        memset(&PRecv, 0, sizeof(PRecv));
        PRecv.Length = Length;
        memcpy(PRecv.Data, buf, BUF_SIZE);
        Mtx_Lock(mtx_Packet);
        Packet_Receive.push(PRecv);
        Mtx_Unlock(mtx_Packet);

        cout << "receive: " << buf << endl;
        Sleep(1);
    }
    return 0;
}


int GenRec()
{
    int i = 0;
    while (true)
    {
        sClient = sock.Accept(sServer);
        if (sClient == INVALID_SOCKET)
        {
            cout << "Accept failed!" << endl;
            return -1;
        }
        else
        {
            cout << "connected" << endl;
            Mtx_Lock(mtx_CSocket);
            Mtx_Lock(mtx_Client);
            CSocket.push_back(sClient);
            Mtx_Unlock(mtx_Client);
            Mtx_Unlock(mtx_CSocket);
            thread Rec(Receiver);
            Rec.detach();
        }
        Sleep(1);
    }
}

int main()
{
    Mtx_Init(Stop, true);
    Mtx_Init(Ender, true);

    if (sock.Init() != 0) //初始化socket
    {
        cout << "WSAStartup failed!" << endl;
        return -1;
    }

    sServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sServer == INVALID_SOCKET)
    {
        cout << "Socket failed!" << endl;
        return -1;
    }

    int retVal = sock.Bind(sServer, 9000, AF_INET);
    if (retVal == -1)
    {
        cout << "bind failed!" << endl;
        closesocket(sServer);   //关闭套接字  
        retVal = 0;
        return -1;
    }
    retVal = sock.Listen(sServer, 1024);
    if (retVal == SOCKET_ERROR)
    {
        cout << "listen failed!" << endl;
        closesocket(sServer);   //关闭套接字  
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
        Sleep(1);
    }
    Mtx_Wait(Stop);
    return 0;
}
