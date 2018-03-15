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

struct Cli_Info
{
    string ip = "0.0.0.0";
    int port = 0;
};

std::mutex mtx;
queue <string> MsgQueue;  //��Ϣ����
vector <Cli_Info> CIP;    //�ͻ���IP

vector <SOCKET> CSocket;    //�ͻ����׽���
HANDLE Stop;
HANDLE Ender;
SOCKET sServer;        //�������׽���  
SOCKET sClient;        //�ͻ����׽���  
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
        mtx.lock();
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
                int retVal = sock.Send(*it, (char*)&Packet_Send, BUF_SIZE + 4);
                if (retVal == -1)
                {
                    cout << "Forward Failed" << endl;
                }
            }
            MsgQueue.pop();
        }
        mtx.unlock();
        Sleep(1);
    }
    return 0;
}


int Receiver()
{

    string Msg;
    Cli_Info CInfo;
    char buf[BUF_SIZE];  //���տͻ������� 
    SOCKADDR_IN Sa_In;
    int len = sizeof(Sa_In);
    std::mutex Locker;

    Locker.lock();
    SOCKET Client = sClient;
    Locker.unlock();

    if (!getpeername(Client, (struct sockaddr *)&Sa_In, &len))
    {
        CInfo.ip = inet_ntoa(Sa_In.sin_addr);
        CInfo.port = ntohs(Sa_In.sin_port);  //IPV6��Ҫʹ��inet_pton()
        mtx.lock();
        CIP.push_back(CInfo);
        mtx.unlock();
    }

    while (true)
    {

        if (WaitForSingleObject(Ender, 0) == WAIT_OBJECT_0)
        {
            cout << "Receiver stopped" << endl;
            return 0;
        }
        int retVal = 0;
        memset(buf, 0, BUF_SIZE);
        int Length = 0;
        retVal = sock.Recv(Client, (char*)&Length, 4);
        retVal = sock.Recv(Client, buf, Length);
        if (retVal == -1)
        {
            cout << "recv failed!" << endl;

            for (unsigned int i = 0; i < CSocket.size(); i++)
            {
                if (CSocket.at(i) == Client)
                {
                    mtx.lock();
                    CSocket.erase(CSocket.begin() + i);
                    CIP.erase(CIP.begin() + i);
                    mtx.unlock();
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
            closesocket(sServer);   //�ر��׽���  
            SetEvent(Stop);
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
        Sleep(1);
    }
    return 0;
}


int GenRec()
{
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
            CSocket.push_back(sClient);
            thread Rec(Receiver);
            Rec.detach();
        }
        Sleep(1);
    }
}

int main()
{
    Stop = CreateEvent(NULL, TRUE, FALSE, FALSE);
    Ender = CreateEvent(NULL, TRUE, FALSE, FALSE);

    if (sock.Init() != 0) //��ʼ��socket
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
        closesocket(sServer);   //�ر��׽���  
        retVal = 0;
        return -1;
    }
    retVal = sock.Listen(sServer, 5);
    if (retVal == SOCKET_ERROR)
    {
        cout << "listen failed!" << endl;
        closesocket(sServer);   //�ر��׽���  
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
            SetEvent(Ender);
        else if (cmd == "start")
            ResetEvent(Ender);
        Sleep(1);
    }
    WaitForSingleObject(Stop, INFINITE);
}
