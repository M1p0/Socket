#include <winsock2.h>
#include <iostream>
#include <string>
#include <thread>
#include <windows.h>
#pragma comment(lib,"Ws2_32.lib")
using namespace std;

const int BUF_SIZE = 64;
SOCKET sHost;
int retVal=0; //����ֵ
string IP;

int SendMsg(SOCKET sHost,char* buf)
{
    retVal = send(sHost, buf, strlen(buf), 0);
    cout << strlen(buf) << endl;
    if (SOCKET_ERROR == retVal)
    {
        cout << "send failed!" << endl;
        closesocket(sHost); //�ر��׽���
        WSACleanup(); //�ͷ��׽�����Դ
        return -1;
    }
}

int Command()
{
    const int Port = 21;  //command port
    WSADATA wsd;
    SOCKADDR_IN SerAddr;
    char buf[BUF_SIZE];
    char bufRecv[BUF_SIZE];

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

    SerAddr.sin_family = AF_INET;
    SerAddr.sin_addr.s_addr = inet_addr(IP.c_str());
    SerAddr.sin_port = htons((short)Port);
    retVal = connect(sHost, (LPSOCKADDR)&SerAddr, sizeof(SerAddr));
    if (SOCKET_ERROR == retVal)
    {
        cout << "connect failed!" << endl;
        closesocket(sHost); //�ر��׽���
        WSACleanup(); //�ͷ��׽�����Դ
        return -1;
    }

    while (true)
    {
        //���������������
        ZeroMemory(buf, BUF_SIZE);
        cout << "Send:";
        cin.getline(buf, BUF_SIZE);//����ʹ��cin,cin��������ո�!
        strcat(buf, "\r\n");//������

        SendMsg(sHost, buf);

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
        ZeroMemory(buf, BUF_SIZE);
        ZeroMemory(bufRecv, BUF_SIZE);
        Sleep(1);
    }
    //�˳�
    closesocket(sHost); //�ر��׽���
    WSACleanup(); //�ͷ��׽�����Դ
    return 0;
}



int main()  //passive mode
{   

    cout << "IP:" << endl;
    cin >> IP;
    thread CMD(Command);
    CMD.detach();

    while (true)
    {
        Sleep(1);
    }
    



}