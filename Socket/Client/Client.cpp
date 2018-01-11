#include <winsock2.h>
#include <iostream>
#include <string>
#include <thread>
#include <windows.h>
#pragma comment(lib,"Ws2_32.lib")
using namespace std;

const int BUF_SIZE = 64;
SOCKET sHost;
HANDLE Rec;
int retVal; //返回值


int Receiver()
{
    while (true)
    {
        char bufRecv[BUF_SIZE]; //接收数据缓冲区 
        ZeroMemory(bufRecv, BUF_SIZE);
        retVal = recv(sHost, bufRecv, BUF_SIZE, 0);
        if (retVal == SOCKET_ERROR)
        {
            cout << "recv failed!" << endl;
        }
        else
        {
            cout << endl;
            cout << "Received:" << bufRecv << endl;
        }
        Sleep(1);
    }
    return 0;
};

int main(int argc, char* argv[])
{
    string IP;
    int Port;

    WSADATA wsd; //WSADATA变量

    SOCKADDR_IN servAddr; //服务器地址
    char buf[BUF_SIZE]; //接收数据缓冲区
    char bufRecv[BUF_SIZE];


    cout << "IP:" << endl;
    cin >> IP;

    cout << "Port:" << endl;
    cin >> Port;
    //IP = "192.168.1.2";
    //Port = 9000;


    //初始化套结字动态库
    if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
    {
        cout << "WSAStartup failed!" << endl;
        return -1;
    }
    //创建套接字
    sHost = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (INVALID_SOCKET == sHost)
    {
        cout << "socket failed!" << endl;
        WSACleanup();//释放套接字资源
        return  -1;
    }


    //设置服务器地址
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = inet_addr(IP.c_str());
    servAddr.sin_port = htons((short)Port);

    //连接服务器

    retVal = connect(sHost, (LPSOCKADDR)&servAddr, sizeof(servAddr));
    if (SOCKET_ERROR == retVal)
    {
        cout << "connect failed!" << endl;
        closesocket(sHost); //关闭套接字
        WSACleanup(); //释放套接字资源
        return -1;
    }

    thread Rec(Receiver);
    Rec.detach();
    while (true)
    {
        //向服务器发送数据
        ZeroMemory(buf, BUF_SIZE);
        cout << "Send:";
        cin.getline(buf, BUF_SIZE);
        retVal = send(sHost, buf, strlen(buf), 0);
        if (SOCKET_ERROR == retVal)
        {
            cout << "send failed!" << endl;
            closesocket(sHost); //关闭套接字
            WSACleanup(); //释放套接字资源
            return -1;
        }
        //RecvLine(sHost, bufRecv);
        ZeroMemory(bufRecv, BUF_SIZE);
        Sleep(100);
    }
    //退出
    closesocket(sHost); //关闭套接字
    WSACleanup(); //释放套接字资源
    return 0;
}


