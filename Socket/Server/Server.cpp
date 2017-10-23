#include <winsock2.h>  
#include <iostream> 
#include <windows.h>
#include <process.h>
#pragma comment(lib,"Ws2_32.lib")
using namespace std;

int main()
{
Loop:
    const int BUF_SIZE = 64;
    WSADATA wsd;            //WSADATA变量  
    SOCKET sServer;        //服务器套接字  
    SOCKET sClient;        //客户端套接字  
    SOCKADDR_IN addrServ;;      //服务器地址  
    char buf[BUF_SIZE];  //接收客户端数据  
    char sendBuf[BUF_SIZE];//发送给客户端的数据  
    int retVal;         //返回值
    if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0) //初始化socket
    {
        cout << "WSAStartup failed!" << endl;
        goto Loop;
    }

    sServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sServer == INVALID_SOCKET)
    {
        cout << "Socket failed!" << endl;
        WSACleanup();
        goto Loop;
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
        goto Loop;
    }

    retVal = listen(sServer, 5);
    if (retVal == SOCKET_ERROR)
    {
        cout << "listen failed!" << endl;
        closesocket(sServer);   //关闭套接字  
        WSACleanup();           //释放套接字资源;  
        goto Loop;
    }

    sockaddr_in addrClient;
    int addrClientlen = sizeof(addrClient);
    sClient = accept(sServer, (sockaddr*)&addrClient, &addrClientlen);
    if (sClient == INVALID_SOCKET)
    {
        cout << "Accept failed!" << endl;
        closesocket(sServer);   //关闭套接字  
        WSACleanup();           //释放套接字资源
        goto Loop;
    }

    while (true)
    {
        ZeroMemory(buf, BUF_SIZE);
        retVal = recv(sClient, buf, BUF_SIZE, 0);
        if (retVal == SOCKET_ERROR)
        {
            cout << "recv failed!" << endl;
            closesocket(sClient);
            closesocket(sServer);
            WSACleanup();
            goto Loop;
        }
        if (buf[0] == '0')
            break;
        cout << "客户端发送的数据: " << buf << endl;

    }
    closesocket(sClient);
    closesocket(sServer);
    WSACleanup();
    goto Loop;
}