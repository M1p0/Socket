#undef  WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include <string>
#include <string.h>
#include <thread>
#include <windows.h>
#include <MSocket.h>
#include <Public.h>
using namespace std;

SOCKET sHost;
MSocket Sock;

int Receiver()
{

    int retVal = 0;
    while (true)
    {
        int Length = 0;
        char Data[BUF_SIZE]; //接收数据缓冲区 
        memset(Data, 0, BUF_SIZE);
        Sock.Recv(sHost, (char*)&Length, 4);
        retVal = Sock.Recv(sHost, Data, Length);
        if (retVal == -1)
        {
            return -1;
        }
        else
        {
            cout << endl;
            cout << "Received:" << Data << endl;
        }
        MSleep(1, "ms");
    }
    return 0;
};

int main()
{
    int retVal;
    string IP;
    int Port;
    IP = "192.168.1.2";
    Port = 9000;

    if (Sock.Init() != 0)      //初始化套结字动态库
    {
        cout << "WSAStartup failed!" << endl;
        return -1;
    }

    sHost = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);      //创建套接字
    if (INVALID_SOCKET == sHost)
    {
        cout << "socket failed!" << endl;
        return  -1;
    }
    retVal = Sock.Connect(sHost, IP.c_str(), Port);      //连接服务器
    if (retVal != 0)
    {
        return -1;
    }

    thread Rec(Receiver);
    Rec.detach();
    string command;
    string cmd;
    while (cin>>cmd)
    {
        Packet Packet_Send;
        memset(&Packet_Send, 0, BUF_SIZE + 4);
        //向服务器发送数据
        cout << "Send:";
        if (cmd == "logon")
        {
            command = R"({"command":"logon","username":"teaaa","password":"password"})";
        }
        if (cmd=="login1")
        {
            command = R"({"command":"login","id":"10000","password":"password"})";
        }
        if (cmd == "login2")
        {
            command = R"({"command":"login","id":"10003","password":"passwd"})";
        }
        if (cmd=="logout")
        {
            command = R"({"command":"logout","id":"10000"})";
        }
        if (cmd == "add")
        {
            command = R"({"command":"add_friend","id":"10000","friend_id":"1","status":"0"})";
        }
        if (cmd == "list")
        {
            command = R"({"command":"list_friend","id":"10000"})";
        }
        if (cmd == "send")
        {
            command = R"({"command":"send_message","src":"10000","dst":"1","message":"中文"})";
        }
        Packet_Send.Length = command.size();
        memcpy(Packet_Send.Data, command.c_str(), command.size());
        retVal = Sock.Send(sHost, (char*)&Packet_Send, command.size() + 4);
        if (SOCKET_ERROR == retVal)
        {
            cout << "send failed!" << endl;
            return -1;
        }
        cin.clear();
        MSleep(1, "ms");
    }
    //退出
    Sock.Close(sHost); //关闭套接字
    return 0;
}


