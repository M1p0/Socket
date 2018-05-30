#include "WebSocket_Server.h"
#include <iostream>
#include <thread>
#include <unordered_map>
#include <string>
#include "MJson.h"
#include <MSocket.h>
#include "MDatabase.h"
#include <MyEvent.h>
#include <Public.h>
using namespace std;


extern mutex mtx_Packet;
extern MDatabase Conn;
extern queue <Packet> Packet_Queue;
unordered_map<string, WS_Info> Map_WS_User;
unordered_map<string, int(*)(const char*, WS_Info)> Map_WS_API;



extern int PushOfflineMessage(const char* id);


int WS_Login(const char* JsonData, WS_Info Info)
{
    string id = "0";
    Document DocReceive;
    Document DocSend;
    DocSend.SetObject();
    int nRow;
    DocReceive.Parse(JsonData);

    if (DocReceive.HasMember("id") && DocReceive.HasMember("password"))
    {
        Value &vid = DocReceive["id"];
        Value &vpassword = DocReceive["password"];
        id = vid.GetString();
        string password = vpassword.GetString();
        string SQL = R"(select username from user where id=")" + id + R"(")" + R"(and password=")" + password + R"(";)";

        vector<vector<string>> Result(1);
        Conn.ExecSQL(SQL.c_str(), Result, nRow);

        if (nRow == 0)   //wrong password
        {
            DocSend.AddMember("command", "login_return", DocSend.GetAllocator());
            DocSend.AddMember("status", "fail", DocSend.GetAllocator());
            DocSend.AddMember("detail", "wrong password or id", DocSend.GetAllocator());
        }
        else
        {
            Map_WS_User.insert(pair<string, WS_Info>(id, Info));
            Value vUsername(Result[0][0].c_str(), DocSend.GetAllocator());
            DocSend.SetObject();
            DocSend.AddMember("command", "login_return", DocSend.GetAllocator());
            DocSend.AddMember("status", "success", DocSend.GetAllocator());
            DocSend.AddMember("username", vUsername, DocSend.GetAllocator());
        }
    }
    else
    {
        DocSend.AddMember("command", "login_return", DocSend.GetAllocator());
        DocSend.AddMember("status", "fail", DocSend.GetAllocator());
        DocSend.AddMember("detail", "wrong json", DocSend.GetAllocator());
    }

    StringBuffer buffer;
    PrettyWriter<StringBuffer> writer(buffer);
    DocSend.Accept(writer);
    string JsonSend = buffer.GetString();
    Info.server->send(Info.hdl, JsonSend.c_str(), websocketpp::frame::opcode::text);
    if (id!="0")
    {
        PushOfflineMessage(id.c_str());
    }
    return 0;
}

int WS_SendMessage(const char* JsonData, WS_Info Info)
{
    Document DocReceive;
    Document DocSend;
    DocSend.SetObject();
    int nRow;
    DocReceive.Parse(JsonData);

    if (DocReceive.HasMember("src") && DocReceive.HasMember("dst") && DocReceive.HasMember("message"))
    {
        Value &vsrc = DocReceive["src"];
        Value &vdst = DocReceive["dst"];
        Value &vmessage = DocReceive["message"];
        string src = vsrc.GetString();
        string dst = vdst.GetString();
        string message = vmessage.GetString();


        unordered_map<string, WS_Info>::iterator it;
        it = Map_WS_User.find(src);
        if (it != Map_WS_User.end())
        {

            string SQL = R"(select * from friendlist where id=")" + src + R"(")" + R"(and friend_id=")" + dst + R"(";)";
            vector<vector<string>> Result(1);
            Conn.ExecSQL(SQL.c_str(), Result, nRow);
            if (nRow == 0)  //无此好友关系
            {
                DocSend.AddMember("command", "send_message_return", DocSend.GetAllocator());
                DocSend.AddMember("status", "fail", DocSend.GetAllocator());
                DocSend.AddMember("detail", "dst not in friend_list", DocSend.GetAllocator());
            }
            else
            {
                Packet Packet_send;
                memset(&Packet_send, 0, BUF_SIZE + 4);
                Packet_send.Length = strlen(JsonData);
                memcpy(Packet_send.Data, JsonData, strlen(JsonData));
                Mtx_Lock(mtx_Packet);
                Packet_Queue.push(Packet_send);
                Mtx_Unlock(mtx_Packet);
                DocSend.AddMember("command", "send_message_return", DocSend.GetAllocator());
                DocSend.AddMember("status", "success", DocSend.GetAllocator());
            }

        }
        else  //未登录
        {
            DocSend.AddMember("command", "send_message_return", DocSend.GetAllocator());
            DocSend.AddMember("status", "fail", DocSend.GetAllocator());
            DocSend.AddMember("detail", "not logged in", DocSend.GetAllocator());
        }

    }
    else  //json错误
    {
        DocSend.AddMember("command", "send_message_return", DocSend.GetAllocator());
        DocSend.AddMember("status", "fail", DocSend.GetAllocator());
        DocSend.AddMember("detail", "wrong json", DocSend.GetAllocator());
    }

    StringBuffer buffer;
    PrettyWriter<StringBuffer> writer(buffer);
    DocSend.Accept(writer);
    string JsonSend = buffer.GetString();
    Info.server->send(Info.hdl, JsonSend.c_str(), websocketpp::frame::opcode::text);
    return 0;

}


void OnOpen(WebsocketServer *server, websocketpp::connection_hdl hdl)
{

    cout << "Web client connected" << endl;
}

void OnClose(WebsocketServer *server, websocketpp::connection_hdl hdl)
{
    unordered_map<string, WS_Info>::iterator it;
    for (it=Map_WS_User.begin();it!=Map_WS_User.end();)
    {
        if (it->second.server==server)
        {
            Map_WS_User.erase(it++);
        }
        else
        {
            it++;
        }
    }
    cout << "Web client disconnected" << endl;
}

void OnMessage(WebsocketServer *server, websocketpp::connection_hdl hdl, message_ptr msg)
{

    string JsonData = msg->get_payload();
    Document DocReceive;
    Document DocSend;
    DocSend.SetObject();
    DocReceive.Parse(JsonData.c_str());
    if (DocReceive.IsObject())
    {
        if (DocReceive.HasMember("command"))
        {
            Value &vcommand = DocReceive["command"];
            string command = vcommand.GetString();
            unordered_map<string, int(*)(const char*, WS_Info)>::iterator it;
            it = Map_WS_API.find(command);
            if (it != Map_WS_API.end())
            {
                WS_Info Info;
                Info.server = server;
                Info.hdl = hdl;
                it->second(JsonData.c_str(), Info);
            }
            else //wrong command
            {
                DocSend.AddMember("command", "return", DocSend.GetAllocator());
                DocSend.AddMember("status", "fail", DocSend.GetAllocator());
                DocSend.AddMember("detail", "wrong command", DocSend.GetAllocator());
                StringBuffer buffer;
                PrettyWriter<StringBuffer> writer(buffer);
                DocSend.Accept(writer);
                string JsonSend = buffer.GetString();
                server->send(hdl, JsonSend.c_str(), websocketpp::frame::opcode::text);
                return;
            }
        }
        else // no command
        {
            DocSend.AddMember("command", "return", DocSend.GetAllocator());
            DocSend.AddMember("status", "fail", DocSend.GetAllocator());
            DocSend.AddMember("detail", "no command", DocSend.GetAllocator());
            StringBuffer buffer;
            PrettyWriter<StringBuffer> writer(buffer);
            DocSend.Accept(writer);
            string JsonSend = buffer.GetString();
            server->send(hdl, JsonSend.c_str(), websocketpp::frame::opcode::text);
            return;
        }
    }
    else  //not object
    {
        DocSend.AddMember("command", "return", DocSend.GetAllocator());
        DocSend.AddMember("status", "fail", DocSend.GetAllocator());
        DocSend.AddMember("detail", "not an Json Object", DocSend.GetAllocator());
        StringBuffer buffer;
        PrettyWriter<StringBuffer> writer(buffer);
        DocSend.Accept(writer);
        string JsonSend = buffer.GetString();
        server->send(hdl, JsonSend.c_str(), websocketpp::frame::opcode::text);
        return;
    }
}



void WS_Run()
{
    WebsocketServer                    server;

    // Set logging settings
    server.set_access_channels(websocketpp::log::alevel::all);
    server.clear_access_channels(websocketpp::log::alevel::frame_payload);

    // Initialize ASIO
    server.init_asio();

    // Register our open handler
    server.set_open_handler(bind(&OnOpen, &server, ::_1));

    // Register our close handler
    server.set_close_handler(bind(&OnClose, &server, _1));

    // Register our message handler
    server.set_message_handler(bind(&OnMessage, &server, _1, _2));

    //Listen on port 2152
    server.listen(9002);

    //Start the server accept loop
    server.start_accept();


    //Start the ASIO io_service run loop
    cout << "WebSocket Server Running..." << endl;
    server.run();
}


