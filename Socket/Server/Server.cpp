#include <iostream>
#include <thread>
#include <string>
#include <string.h>
#include <queue>
#include <vector>
#include <mutex>
#include <MyEvent.h>
#include <MSocket.h>
#include <Public.h>
#pragma comment(lib,"Lib.lib")
using namespace std;

std::mutex mtx_CIP;
std::mutex mtx_CSocket;
std::mutex mtx_sClient;
std::mutex mtx_MsgQue;
std::mutex mtx_Packet;
queue <Packet> Packet_Queue;
vector <Cli_Info> CIP;    //客户端IP
vector <SOCKET> CSocket;    //客户端套接字
std::mutex  Stop;
std::mutex Ender;
SOCKET sServer;        //服务器套接字  
SOCKET sClient;        //客户端套接字  
MSocket sock;

int Certificate(SOCKET Client)
{
    int Length;
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
        if (Packet_Queue.size() != 0 && CSocket.size() != 0)
        {
            vector<SOCKET>::iterator it;
            for (it = CSocket.begin(); it != CSocket.end(); it++)
            {
                Packet Packet_Send;
                Packet_Send = Packet_Queue.front();
                int retVal = sock.Send(*it, (char*)&Packet_Send, BUF_SIZE + 4); //阻塞式send 待修改
                if (retVal == -1)
                {
                    cout << "Forward Failed" << endl;
                }
            }
            Packet_Queue.pop();
        }
        Mtx_Unlock(mtx_Packet);
        Mtx_Unlock(mtx_CSocket);
        MSleep(1, "ms");
    }
    return 0;
}


int Receiver()
{
    string Msg;
    Cli_Info CInfo;
    Mtx_Lock(mtx_sClient);
    SOCKET Client = sClient;
    Mtx_Unlock(mtx_sClient);

    //if (Certificate(Client) != 0)
    //{
    //    Mtx_Lock(mtx_CSocket);
    //    for (unsigned int i = 0; i < CSocket.size(); i++)
    //    {
    //        if (CSocket.at(i) == Client)
    //        {
    //            CSocket.erase(CSocket.begin() + i);
    //        }
    //    }
    //    Mtx_Unlock(mtx_CSocket);
    //    sock.Close(Client);
    //    return -1;
    //}

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
        int Length = 0;
        char buf[BUF_SIZE];  //接收客户端数据 
        memset(buf, 0, BUF_SIZE);
        retVal = sock.Recv(Client, (char*)&Length, 4);

        printf("Length:0x%x  ", Length);

        retVal = sock.Recv(Client, (char*)buf, Length);
        printf("Data:");
        for (int i = 0; i < 20; i++)
        {
            printf("0x%02x ", buf[i]);
        }
        cout << endl;
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
            sock.Close(Client);
            return -1;
        }
        if (buf[0] == '0')
            break;
        Packet PRecv;
        memset(&PRecv, 0, sizeof(PRecv));
        PRecv.Length = Length;
        memcpy(PRecv.Data, buf, BUF_SIZE);
        Mtx_Lock(mtx_Packet);
        Packet_Queue.push(PRecv);
        Mtx_Unlock(mtx_Packet);


        cout << "receive: " << buf << endl;
        MSleep(1, "ms");
    }
    return 0;
}

int GenRec()
{
    while (true)
    {
        sClient = sock.Accept(sServer);
        if (sClient == -1)
        {
            cout << "Accept failed!" << endl;
            return -1;
        }
        else
        {
            cout << "connected" << endl;
            Mtx_Lock(mtx_CSocket);
            Mtx_Lock(mtx_sClient);
            CSocket.push_back(sClient);
            Mtx_Unlock(mtx_sClient);
            Mtx_Unlock(mtx_CSocket);
            thread Rec(Receiver);
            Rec.detach();
        }
        MSleep(1, "ms");
    }
}


int main()
{
    int retVal = 0;
    Mtx_Init(Stop, true);
    Mtx_Init(Ender, true);
    sServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    CSocket.reserve(1000);
    if (sServer == -1)
    {
        cout << "Socket failed!" << endl;
        sock.Close(sServer);
        return -1;
    }

    retVal = sock.Bind(sServer, 9000, AF_INET);
    if (retVal == -1)
    {
        cout << "bind failed!" << endl;
        sock.Close(sServer);
        retVal = 0;
        return -1;
    }
    retVal = sock.Listen(sServer, 5);
    if (retVal == -1)
    {
        cout << "listen failed!" << endl;
        sock.Close(sServer);
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
        MSleep(1, "ms");
    }

    Mtx_Wait(Stop);
    return 0;
}

