#include <winsock2.h>
#include <iostream>
#include <string>
#include <thread>
#include <windows.h>
#pragma comment(lib,"Ws2_32.lib")
using namespace std;

const int BUF_SIZE = 512;
SOCKET sHost;
HANDLE Rec;
int retVal; //����ֵ

#pragma pack(1)
struct SPacket
{
    int Length;
    char Data[BUF_SIZE];
};
#pragma pack()



int Receiver()
{
    while (true)
    {
        char bufRecv[BUF_SIZE]; //�������ݻ����� 
        memset(bufRecv, 0, BUF_SIZE);
        retVal = recv(sHost, bufRecv, BUF_SIZE, 0);
        if (retVal == SOCKET_ERROR)
        {
            cout << "recv failed!" << endl;
            return -1;
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

void Certificate(SOCKET Server)
{
    SPacket Packet_Send;

    char buf[BUF_SIZE];
    memset(buf, 0, BUF_SIZE);
    memcpy(buf, "root", 4);
    memset(&Packet_Send, 0, BUF_SIZE + 4);
    Packet_Send.Length = BUF_SIZE;
    memcpy(Packet_Send.Data, buf, BUF_SIZE);
    send(sHost, (char*)&Packet_Send, BUF_SIZE + 4, 0);
}

int main(int argc, char* argv[])
{
    string IP;
    int Port;

    WSADATA wsd; //WSADATA����

    SOCKADDR_IN servAddr; //��������ַ


    //cout << "IP:" << endl;
    //cin >> IP;

    //cout << "Port:" << endl;
    //cin >> Port;
    IP = "172.105.202.158";
    //IP = "192.168.1.7";
    Port = 9000;


    //��ʼ���׽��ֶ�̬��
    if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
    {
        cout << "WSAStartup failed!" << endl;
        return -1;
    }
    //�����׽���
    sHost = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (INVALID_SOCKET == sHost)
    {
        cout << "socket failed!" << endl;
        WSACleanup();//�ͷ��׽�����Դ
        return  -1;
    }


    //���÷�������ַ
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = inet_addr(IP.c_str());
    servAddr.sin_port = htons((short)Port);

    //���ӷ�����

    retVal = connect(sHost, (LPSOCKADDR)&servAddr, sizeof(servAddr));
    if (SOCKET_ERROR == retVal)
    {
        cout << "connect failed!" << endl;
        closesocket(sHost); //�ر��׽���
        WSACleanup(); //�ͷ��׽�����Դ
        return -1;
    }

    thread Rec(Receiver);
    Rec.detach();
    Certificate(sHost);
    while (true)
    {
        SPacket Packet_Send;
        //���������������
        cout << "Send:";
        char buf[BUF_SIZE];
        memset(buf, 0, BUF_SIZE);
        cin.getline(buf, BUF_SIZE);

        memset(&Packet_Send, 0, BUF_SIZE + 4);
        Packet_Send.Length = BUF_SIZE;
        memcpy(Packet_Send.Data, buf, BUF_SIZE);
        cout << "sizeof Packet:" << sizeof(Packet_Send) << endl;
        retVal = send(sHost, (char*)&Packet_Send, BUF_SIZE + 4, 0);
        if (SOCKET_ERROR == retVal)
        {
            cout << "send failed!" << endl;
            closesocket(sHost); //�ر��׽���
            WSACleanup(); //�ͷ��׽�����Դ
            return -1;
        }
        cin.clear();
        Sleep(1);
    }
    //�˳�
    closesocket(sHost); //�ر��׽���
    WSACleanup(); //�ͷ��׽�����Դ
    return 0;
}


