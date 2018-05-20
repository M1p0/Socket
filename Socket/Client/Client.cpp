#undef  WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include <string>
#include <string.h>
#include <thread>
#include <windows.h>
#include <MSocket.h>
#include <Public.h>
#pragma comment(lib,"Lib.lib")
using namespace std;

SOCKET sHost;
MSocket sock;

int Receiver()
{

    int retVal = 0;
    while (true)
    {
        int Length = 0;
        char Data[BUF_SIZE]; //�������ݻ����� 
        memset(Data, 0, BUF_SIZE);
        sock.Recv(sHost, (char*)&Length, 4);
        retVal = sock.Recv(sHost, Data, Length);
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

    if (sock.Init() != 0)      //��ʼ���׽��ֶ�̬��
    {
        cout << "WSAStartup failed!" << endl;
        return -1;
    }

    sHost = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);      //�����׽���
    if (INVALID_SOCKET == sHost)
    {
        cout << "socket failed!" << endl;
        return  -1;
    }
    retVal = sock.Connect(sHost, IP.c_str(), Port);      //���ӷ�����
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
        //���������������
        cout << "Send:";
        if (cmd=="login")
        {
            command = R"({"command":"login","id":"10000","password":"password"})";
        }
        if (cmd=="logout")
        {
            command = R"({"command":"logout","id":"10000"})";
        }
        if (cmd == "add")
        {
            command = R"({"command":"add_friend","id":"10000","friend_id":"10003","status":"0"})";
        }
        if (cmd == "list")
        {
            command = R"({"command":"list_friend","id":"10000"})";
        }

        Packet_Send.Length = command.size();
        memcpy(Packet_Send.Data, command.c_str(), command.size());
        retVal = sock.Send(sHost, (char*)&Packet_Send, command.size() + 4);
        if (SOCKET_ERROR == retVal)
        {
            cout << "send failed!" << endl;
            return -1;
        }
        cin.clear();
        MSleep(1, "ms");
    }
    //�˳�
    sock.Close(sHost); //�ر��׽���
    return 0;
}


