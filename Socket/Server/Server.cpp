#undef  WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <unordered_map>
#include <thread>
#include <string>
#include <string.h>
#include <queue>
#include <vector>
#include <mutex>
#include <MyEvent.h>
#include <MSocket.h>
#include <Public.h>
#include <MDatabase.h>
#include "Message.h"
#include "MJson.h"
#include "Utils.h"
#include "CFileIO.h"
#include "Http_Server.h"
using namespace std;

#ifdef _WIN32
#pragma comment(lib,"Lib.lib")
#endif


mutex mtx_CIP;
mutex mtx_CSocket;
mutex mtx_sClient;
mutex mtx_MsgQue;
mutex mtx_Packet;
mutex mtx_Map_User;
queue <Packet> Packet_Queue;
vector <Cli_Info> CIP;    //客户端IP
vector <SOCKET> CSocket;    //客户端套接字
SOCKET sServer;        //服务器套接字  
SOCKET sClient;        //客户端套接字  
MSocket sock;
MDatabase Conn;
CFileIO File;
unordered_map<string, int(*)(const char*, SOCKET)> Map_Func;
unordered_map<string, SOCKET> Map_User;

int Forward()
{
    cout << "Forward running" << endl;
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
    Mtx_Lock(mtx_sClient);
    SOCKET Client = sClient;
    Mtx_Unlock(mtx_sClient);
    string Msg;
    Cli_Info CInfo;

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
        int retVal = 0;
        int Length = 0;
        char buf[BUF_SIZE];  //接收客户端数据 
        memset(buf, 0, BUF_SIZE);
        retVal = sock.Recv(Client, (char*)&Length, 4);

        printf("Length:0x%x  ", Length);

        retVal = sock.Recv(Client, (char*)buf, Length);
        printf("Data:");
        for (int i = 0; i < Length; i++)
        {
            printf("0x%02x ", buf[i]);
        }
        cout << endl;
        cout << "Data_String:" << buf << endl;
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

        //获取指令
        Document document;
        document.Parse(buf);
        if (document.IsObject())
        {
            if (document.HasMember("command"))
            {
                Value &value1 = document["command"];
                string command = value1.GetString();
                unordered_map<string, int(*)(const char*, SOCKET)>::iterator it;
                it = Map_Func.find(command);
                if (it!=Map_Func.end())
                {
                    it->second(buf, Client);
                }
                else
                {
                    cout << "command not found" << endl;
                }
            }
        }
        MSleep(1, "ms");
    }
    return 0;
}

int GenRec()
{
    cout << "GenRec running" << endl;
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
    InitMap();
    int retVal = 0;
    sServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    CSocket.reserve(1024);

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

    if (Conn.Connect("192.168.1.2", "root", "admin", "myim", 3306) != 0)
    {
        cout << "Database connect failed" << endl;
        return -1;
    }
    else
    {
        cout << "Database connect succeed" << endl;
    }

    thread Gen(GenRec);
    thread Fwd(Forward);
    //while (true)
    //{
    //    string cmd;
    //    cin >> cmd;
    //    if (cmd=="list")
    //    {
    //        unordered_map<string, SOCKET>::iterator it;
    //        for (it=Map_User.begin();it!=Map_User.end();it++)
    //        {
    //            cout << "id:" << it->first << endl;
    //        }
    //    }
    //    MSleep(10, "ms");
    //}
    startHttpServer("0.0.0.0", 9001, MyHttpServerHandler, NULL);
    Gen.join();
    Fwd.join();
    return 0;
}

