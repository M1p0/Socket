#include <winsock2.h>
#include <iostream>
#include <string>
#include <process.h>
#include <windows.h>
#pragma comment(lib,"Ws2_32.lib")
using namespace std;
const int BUF_SIZE = 64;
unsigned __stdcall Command(void *p)
{
    const int Port = 21;//命令端口
    int retVal;
    WSADATA wsd;
    SOCKET sServer;
    SOCKET sClient;
    SOCKADDR_IN addrSer; //服务器地址
    SOCKADDR_IN addrCli;
    char bufRecv[BUF_SIZE]; //接收消息

    if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
    {
        cout << "WSAStartup Failed" << endl;
        return -1;
    }

    sServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sServer == INVALID_SOCKET)
    {
        cout << "Socket Failed" << endl;
        WSACleanup();
        return -1;
    }

    addrSer.sin_family = AF_INET;
    addrSer.sin_port = htons((short)Port);  //FTP命令端口
    addrSer.sin_addr.s_addr = INADDR_ANY;
    retVal = ::bind(sServer, (LPSOCKADDR)&addrSer, sizeof(addrSer));

    if (retVal == SOCKET_ERROR)
    {
        cout << "Bind Failed" << endl;
        closesocket(sServer);
        WSACleanup();
        retVal = 0;
        return -1;
    }

    retVal = listen(sServer, 5);
    if (retVal == SOCKET_ERROR)
    {
        cout << "Listen Failed" << endl;
        closesocket(sServer);
        WSACleanup();
        retVal = 0;
        return -1;
    }

    int addrClilen = sizeof(addrCli);
    sClient = accept(sServer, (sockaddr*)&addrCli, &addrClilen);
    if (sClient == INVALID_SOCKET)
    {
        cout << "Accept Failed!" << endl;
        closesocket(sServer);
        closesocket(sClient);
        WSACleanup();
        return -1;
    }

    else
    {
        while (true)
        {
            ZeroMemory(bufRecv, BUF_SIZE);
            retVal = recv(sClient, bufRecv, BUF_SIZE, 0);
            if (retVal == SOCKET_ERROR)
                continue;

            cout << bufRecv << endl;

            string Msg = "welcome";
            send(sClient, Msg.c_str(), Msg.size(), 0);
            Sleep(1);
        }


    }



};



int main()  //passive mode
{
    HANDLE CMD;
    CMD = (HANDLE)_beginthreadex(NULL, 0, &Command, NULL, 0, NULL);
    

    while (true)
    {
        Sleep(1);
    }

}