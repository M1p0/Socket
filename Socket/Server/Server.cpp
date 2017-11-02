#include <winsock2.h>  
#include <iostream> 
#include <windows.h>
#include <process.h>
#include <string>
#include <queue>
#include <vector>
#include <mutex>
#pragma comment(lib,"Ws2_32.lib")
using namespace std;

struct Cli_Info
{
    string ip = "0.0.0.0";
    int port = 0;
};

std::mutex mtx;
CRITICAL_SECTION gSection;
const int BUF_SIZE = 64;

HANDLE Rec;
HANDLE Gen;
HANDLE Stop;
HANDLE Fwd;
SOCKET sServer;        //服务器套接字  
SOCKET sClient;        //客户端套接字  
int retVal;         //返回值
queue <string> MsgQueue;  //消息队列
vector <SOCKET> CSocket;    //客户端套接字
vector <Cli_Info> CIP;    //客户端IP


unsigned __stdcall Forward(void *p)
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
                send(*it, Msg.c_str(), Msg.size(), 0);
            }
            MsgQueue.pop();
        }
        mtx.unlock();
        Sleep(1);
    }
    return 0;
}


unsigned __stdcall Receiver(void *p)
{

    string Msg;
    Cli_Info CInfo;
    SOCKET Client = sClient;
    char buf[BUF_SIZE];  //接收客户端数据 

    struct sockaddr_in Sa_In;
    int len = sizeof(Sa_In);
    if (!getpeername(Client, (struct sockaddr *)&Sa_In, &len))
    {
        CInfo.ip = inet_ntoa(Sa_In.sin_addr);
        CInfo.port = ntohs(Sa_In.sin_port);
        mtx.lock();
        CIP.push_back(CInfo);
        mtx.unlock();
    }

    while (true)
    {
        ZeroMemory(buf, BUF_SIZE);
        retVal = recv(Client, buf, BUF_SIZE, 0);
        if (retVal == SOCKET_ERROR)
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

            _endthreadex(-1);
        }
        if (buf[0] == '0')
            break;
        Msg = buf;
        cout << "receive: " << Msg << endl;
        MsgQueue.push(Msg);


        if (Msg == "root@admin")
        {
            closesocket(sServer);   //关闭套接字  
            WSACleanup();           //释放套接字资源
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
    _endthreadex(0);
    return 0;
}


unsigned __stdcall GenRec(void *p)
{

    sockaddr_in addrClient;
    int addrClientlen = sizeof(addrClient);

    while (true)
    {
        sClient = accept(sServer, (sockaddr*)&addrClient, &addrClientlen);
        if (sClient == INVALID_SOCKET)
        {
            cout << "Accept failed!" << endl;
            _endthreadex(-1);
        }
        else
        {
            cout << "connected" << endl;
            CSocket.push_back(sClient);
            Rec = (HANDLE)_beginthreadex(NULL, 0, &Receiver, NULL, 0, NULL);
        }
        Sleep(1);
    }
}

int main()
{
    WSADATA wsd;            //WSADATA变量  
    SOCKADDR_IN addrServ;      //服务器地址  
    InitializeCriticalSection(&gSection);
    if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0) //初始化socket
    {
        cout << "WSAStartup failed!" << endl;
        return -1;
    }

    sServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sServer == INVALID_SOCKET)
    {
        cout << "Socket failed!" << endl;
        WSACleanup();
        return -1;
    }

    addrServ.sin_family = AF_INET;
    addrServ.sin_port = htons(9000);
    addrServ.sin_addr.s_addr = INADDR_ANY;
    retVal = ::bind(sServer, (LPSOCKADDR)&addrServ, sizeof(addrServ));
    if (retVal == SOCKET_ERROR)
    {
        cout << "bind failed!" << endl;
        closesocket(sServer);   //关闭套接字  
        WSACleanup();           //释放套接字资源;  
        return -1;
    }
    retVal = listen(sServer, 5);
    if (retVal == SOCKET_ERROR)
    {
        cout << "listen failed!" << endl;
        closesocket(sServer);   //关闭套接字  
        WSACleanup();           //释放套接字资源;  
        return -1;
    }
    Gen = (HANDLE)_beginthreadex(NULL, 0, &GenRec, NULL, 0, NULL);
    Fwd = (HANDLE)_beginthreadex(NULL, 0, &Forward, NULL, 0, NULL);
    Stop = CreateEvent(NULL, TRUE, FALSE, FALSE);

    WaitForSingleObject(Stop, INFINITE);

}
