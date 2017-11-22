#include <winsock2.h>
#include <iostream>
#include <string>
#include <process.h>
#include <windows.h>
#pragma comment(lib,"Ws2_32.lib")
using namespace std;

const int BUF_SIZE = 64;
SOCKET sHost;
int retVal; //����ֵ
string IP;

unsigned __stdcall Command(void *p)
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
        cout << "Send:  ";
        cin >> buf;
        retVal = send(sHost, buf, strlen(buf), 0);
        if (SOCKET_ERROR == retVal)
        {
            cout << "send failed!" << endl;
            closesocket(sHost); //�ر��׽���
            WSACleanup(); //�ͷ��׽�����Դ
            return -1;
        }

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
    //�˳�
    closesocket(sHost); //�ر��׽���
    WSACleanup(); //�ͷ��׽�����Դ
    return 0;

}



int main()  //passive mode
{   
    HANDLE CMD;
    cout << "IP:" << endl;
    cin >> IP;
    CMD = (HANDLE)_beginthreadex(NULL, 0, &Command, NULL, 0, NULL);

    while (true)
    {
        Sleep(1);
    }
    



}