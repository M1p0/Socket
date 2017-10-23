#include <winsock2.h>  
#include <iostream> 
#include <windows.h>
#include <process.h>
#include <string>
#pragma comment(lib,"Ws2_32.lib")
using namespace std;

const int BUF_SIZE = 64;

HANDLE Rec;
HANDLE Gen;
HANDLE Stop;
SOCKET sServer;        //服务器套接字  
SOCKET sClient;        //客户端套接字  
int retVal;         //返回值




unsigned __stdcall Receiver(void *p)
{

    string cmd;
    char buf[BUF_SIZE];  //接收客户端数据  
    while (true)
    {
        ZeroMemory(buf, BUF_SIZE);
        retVal = recv(sClient, buf, BUF_SIZE, 0);
        if (retVal == SOCKET_ERROR)
        {
            cout << "recv failed!" << endl;
            _endthreadex(-1);
        }
        if (buf[0] == '0')
            break;
        cmd = buf;
        cout << "receive: " << cmd << endl;

        if (cmd == "root@admin")
        {
            closesocket(sServer);   //关闭套接字  
            closesocket(sClient);
            WSACleanup();           //释放套接字资源
            SetEvent(Stop);
            return 0;
        }
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
            Rec = (HANDLE)_beginthreadex(NULL, 0, &Receiver, NULL, 0, NULL);
        }
    }
}

int main()
{
    WSADATA wsd;            //WSADATA变量  
    SOCKADDR_IN addrServ;;      //服务器地址  
    char sendBuf[BUF_SIZE];//发送给客户端的数据  


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
    retVal = bind(sServer, (LPSOCKADDR)&addrServ, sizeof(addrServ));
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
    Stop = CreateEvent(NULL, TRUE, FALSE, FALSE);
    WaitForSingleObject(Stop,INFINITE);

}
