#pragma once
//注意winsock2.h和windows.h的冲突 
//#undef  WIN32_LEAN_AND_MEAN
//#define WIN32_LEAN_AND_MEAN
#include <string>
#define BUF_SIZE 2048

#ifdef _WIN32
#undef  WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#pragma comment(lib,"Ws2_32.lib")

#else
#include <netinet/in.h> //套接字地址结构
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#define SOCKET int
#endif 

#pragma pack(1)
struct Packet
{
    int Length;
    char Data[BUF_SIZE];
};

struct Cli_Info
{
    std::string ip = "0.0.0.0";
    int port = 0;
};
#pragma pack()

class MSocket
{
public:
    MSocket();
    ~MSocket();
    int Init();
    int Send(SOCKET s, const char* Msg, int Length);
    int Recv(SOCKET s, char* Msg, int Length);
    int Connect(SOCKET s, const char *Name, int Port, int Family = AF_INET);
    int Bind(SOCKET s, int Port, int Family = AF_INET);
    int Listen(SOCKET s, int Backlog);
    int Close(SOCKET s);
    SOCKET Accept(SOCKET s);
    int Getpeername(SOCKET Client, Cli_Info &CInfo);
private:
#ifdef _WIN32
    WSADATA wsd;
#endif
};


