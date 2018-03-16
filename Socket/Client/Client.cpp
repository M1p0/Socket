#undef  WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include <string>
#include <thread>
#include <windows.h>
#include <MSocket.h>
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
        Sleep(1);
    }
    return 0;
};

void Certificate(SOCKET Server)
{
    Packet Packet_Send;

    char buf[BUF_SIZE];
    memset(buf, 0, BUF_SIZE);
    memcpy(buf, "root", 4);
    memset(&Packet_Send, 0, BUF_SIZE + 4);
    Packet_Send.Length = BUF_SIZE;
    memcpy(Packet_Send.Data, buf, BUF_SIZE);
    sock.Send(sHost, (char*)&Packet_Send, BUF_SIZE + 4);
}

int main()
{
    int retVal;
    string IP;
    int Port;

    //cout << "IP:" << endl;
    //cin >> IP;
    //cout << "Port:" << endl;
    //cin >> Port;
    //IP = "172.104.85.54";
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
    if (retVal!=0)
    {
        return -1;
    }

    thread Rec(Receiver);
    Rec.detach();
    Certificate(sHost);
    while (true)
    {
        Packet Packet_Send;
        //���������������
        cout << "Send:";
        char buf[BUF_SIZE];
        memset(buf, 0, BUF_SIZE);
        cin.getline(buf, BUF_SIZE);

        memset(&Packet_Send, 0, BUF_SIZE + 4);
        Packet_Send.Length = BUF_SIZE;
        memcpy(Packet_Send.Data, buf, BUF_SIZE);
        cout << "sizeof Packet:" << sizeof(Packet_Send) << endl;
        retVal = sock.Send(sHost, (char*)&Packet_Send, BUF_SIZE+4);
        if (SOCKET_ERROR == retVal)
        {
            cout << "send failed!" << endl;
            return -1;
        }
        cin.clear();
        Sleep(1);
    }
    //�˳�
    closesocket(sHost); //�ر��׽���
    return 0;
}


