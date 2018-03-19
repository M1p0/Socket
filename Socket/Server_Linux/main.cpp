#include <iostream>
#include <thread>
#include <string>
#include <string.h>
#include <queue>
#include <vector>
#include <mutex>
#include <MyEvent.h>
#include <MSocket.h>
#define SOCKET int
using namespace std;

const int MAX_SIZE = 1024;
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
                Packet Packet_Send;
                memset(&Packet_Send, 0, BUF_SIZE + 4);
                Packet_Send.Length = BUF_SIZE;
                memcpy(Packet_Send.Data, Msg.c_str(), Msg.length());
                sock.Send(*it, (char*)&Packet_Send, BUF_SIZE + 4);
            }
            MsgQueue.pop();
        }
        Mtx_Unlock(mtx_CSocket);
        sleep(1);
    }
    return 0;
}


int Receiver()
{
    string Msg;
    Cli_Info CInfo;
    SOCKET Client = sClient;

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
        close(Client);
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

        int Length = 0;
        char Data[MAX_SIZE];
        memset(Data, 0, MAX_SIZE);
        retVal = sock.Recv(Client,(char*) &Length, 4);
        retVal = sock.Recv(Client, (char*)Data, Length);

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
        MsgQueue.push(Msg);


        if (Msg == "show")
        {
            vector<Cli_Info>::iterator it;
            for (it = CIP.begin(); it != CIP.end(); it++)
            {
                cout << it->ip << ":" << it->port << endl;
            }
        }
        Sleep(1);
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

    retVal = sock.Bind(sServer,9000, AF_INET);
    if (retVal == -1)
    {
        cout << "bind failed!" << endl;
        close(sServer);   //关闭套接字    
        retVal = 0;
        return -1;
    }
    retVal = sock.Listen(sServer, 5);
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

