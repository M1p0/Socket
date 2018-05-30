#undef  WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <unordered_map>
#include <thread>
#include <string>
#include "WebSocket_Server.h"
#include <string.h>
#include <queue>
#include <vector>
#include <MyEvent.h>
#include <MSocket.h>
#include <Public.h>
#include <MDatabase.h>
#include "Message.h"
#include "MJson.h"
#include "Utils.h"
#include "CFileIO.h"
#include "Http_Server.h"
using namespace std;

mutex mtx_sClient;
mutex mtx_Packet;
mutex mtx_Map_User;
queue <Packet> Packet_Queue;
MSocket Sock;
MDatabase Conn;
CFileIO File;
unordered_map<string, int(*)(const char*, SOCKET)> Map_Func;
unordered_map<string, SOCKET> Map_User;
extern unordered_map<string, WS_Info> Map_WS_User;

int Forward()
{
    cout << "Forward Server running..." << endl;
    while (true)
    {
        Mtx_Lock(mtx_Packet);
        if (Packet_Queue.size() != 0)
        {
            string Data = Packet_Queue.front().Data;
            Document document;
            document.Parse(Data.c_str());

            if (document.HasMember("src") && document.HasMember("dst") && document.HasMember("message"))
            {
                Value &vsrc = document["src"];
                Value &vdst = document["dst"];
                Value &vmessage = document["message"];
                string src = vsrc.GetString();
                string dst = vdst.GetString();
                string message = vmessage.GetString();
                unordered_map<string, SOCKET>::iterator it;
                Mtx_Lock(mtx_Map_User);
                it = Map_User.find(dst);
                if (it != Map_User.end())  //android����
                {
                    Sock.Send(it->second, (char*)&Packet_Queue.front(), Packet_Queue.front().Length + 4);
                    Packet_Queue.pop();
                    Mtx_Unlock(mtx_Packet);
                }
                else  //web����
                {
                    unordered_map<string, WS_Info>::iterator it;
                    it = Map_WS_User.find(dst);
                    if (it != Map_WS_User.end())
                    {
                        it->second.server->send(it->second.hdl, Packet_Queue.front().Data, websocketpp::frame::opcode::text);
                        Packet_Queue.pop();
                        Mtx_Unlock(mtx_Packet);
                    }
                    else //д�����ݿ�
                    {
                        vector<vector<string>> Result(1);
                        int nRow = 0;
                        string SQL = R"(insert into offline_message values(")" + src + R"(",")" + dst + R"(",")" + message + R"(");)";
                        Conn.ExecSQL(SQL.c_str(), Result, nRow);
                        Packet_Queue.pop();
                        Mtx_Unlock(mtx_Packet);
                    }
                }
                Mtx_Unlock(mtx_Map_User);
            }
            else//json������ ��src/dst/message
            {
                Packet_Queue.pop();
                Mtx_Unlock(mtx_Packet);
            }
        }
        else   //not json
        {
            Mtx_Unlock(mtx_Packet);
        }
        MSleep(1, "ms");
    }
    return 0;
}


int Receiver(SOCKET sClient)
{
    string Msg;
    Cli_Info CInfo;

    while (true)
    {
        int retVal = 0;
        int Length = 0;
        char buf[BUF_SIZE];  //���տͻ������� 
        memset(buf, 0, BUF_SIZE);
        retVal = Sock.Recv(sClient, (char*)&Length, 4);

        printf("Length:0x%x  ", Length);

        retVal = Sock.Recv(sClient, (char*)buf, Length);
        printf("Data:");
        for (int i = 0; i < Length; i++)
        {
            printf("0x%02x ", buf[i]);
        }
        cout << endl;
        cout << "Data_String:" << buf << endl;
        if (retVal <= 0)
        {
            cout << "Android client disconnected" << endl;
            //��map��ɾ��
            unordered_map<string, SOCKET>::iterator it;
            for (it = Map_User.begin(); it != Map_User.end(); )
            {
                if (it->second == sClient)
                {
                    Map_User.erase(it++);
                }
                else
                {
                    it++;
                }
            }
            Sock.Close(sClient);
            return -1;
        }
        if (buf[0] == '0')
            break;
        Packet PRecv;
        memset(&PRecv, 0, sizeof(PRecv));
        PRecv.Length = Length;
        memcpy(PRecv.Data, buf, BUF_SIZE);

        //��ȡָ��
        Document document;
        document.Parse(buf);
        if (document.IsObject())
        {
            if (document.HasMember("command"))
            {
                Value &value1 = document["command"];
                string command = value1.GetString();
                unordered_map<string, int(*)(const char*, SOCKET)>::iterator it;
                it = Map_Func.find(command);
                if (it != Map_Func.end())
                {
                    it->second(buf, sClient);
                }
                else    //�����json�� command����ȷ
                {
                    cout << "command not found" << endl;
                }
            }
        }
        MSleep(1, "ms");
    }
    return 0;
}

int Listener(SOCKET sServer)
{
    cout << "Listener running..." << endl;
    while (true)
    {
        SOCKET sClient = Sock.Accept(sServer);
        if (sClient == -1)
        {
            cout << "Accept failed!" << endl;
            return -1;
        }
        else
        {
            cout << "Android client connected" << endl;
            thread Rec(Receiver, sClient);
            Rec.detach();
        }
        MSleep(1, "ms");
    }
}


int main()
{

    InitMap();
    Create_Type_Map();
    int retVal = 0;
    SOCKET sServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    retVal = Sock.Bind(sServer, 9000, AF_INET);
    if (retVal == -1)
    {
        cout << "bind failed!" << endl;
        Sock.Close(sServer);
        retVal = 0;
        return -1;
    }
    retVal = Sock.Listen(sServer, 5);
    if (retVal == -1)
    {
        cout << "listen failed!" << endl;
        Sock.Close(sServer);
        retVal = 0;
        return -1;
    }

    if (Conn.Connect("192.168.1.2", "root", "admin", "myim", 3306) != 0)
    {
        cout << "Database connect failed" << endl;
        return -1;
    }
    else
    {
        cout << "Database connect succeed" << endl;
    }

    thread Gen(Listener, sServer);
    MSleep(10, "ms");
    thread Fwd(Forward);
    MSleep(10, "ms");
    thread WS_Server(WS_Run);
    MSleep(10, "ms");
    startHttpServer("0.0.0.0", 9001, MyHttpServerHandler, NULL);
    WS_Server.join();
    Gen.join();
    Fwd.join();
    return 0;
}


