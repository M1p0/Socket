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
int retVal; //����ֵ


int Receiver()
{
    while (true)
    {
        char bufRecv[BUF_SIZE]; //�������ݻ����� 
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

    WSADATA wsd; //WSADATA����

    SOCKADDR_IN servAddr; //��������ַ
    char buf[BUF_SIZE]; //�������ݻ�����
    char bufRecv[BUF_SIZE];


    cout << "IP:" << endl;
    cin >> IP;

    cout << "Port:" << endl;
    cin >> Port;
    //IP = "192.168.1.2";
    //Port = 9000;


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
    while (true)
    {
        //���������������
        ZeroMemory(buf, BUF_SIZE);
        cout << "Send:";
        cin.getline(buf, BUF_SIZE);
        retVal = send(sHost, buf, strlen(buf), 0);
        if (SOCKET_ERROR == retVal)
        {
            cout << "send failed!" << endl;
            closesocket(sHost); //�ر��׽���
            WSACleanup(); //�ͷ��׽�����Դ
            return -1;
        }
        //RecvLine(sHost, bufRecv);
        ZeroMemory(bufRecv, BUF_SIZE);
        Sleep(100);
    }
    //�˳�
    closesocket(sHost); //�ر��׽���
    WSACleanup(); //�ͷ��׽�����Դ
    return 0;
}


