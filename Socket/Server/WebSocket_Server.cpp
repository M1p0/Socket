#include "WebSocket_Server.h"
#include <iostream>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include "MJson.h"
#include <MSocket.h>
#include "MDatabase.h"
#include <MyEvent.h>
#include <Public.h>
using namespace std;


extern mutex mtx_Packet;
extern mutex mtx_Map_AddFriend;
extern MDatabase Conn;
extern queue <Packet> Packet_Queue;
unordered_map<string, WS_Info> Map_WS_User;
unordered_map<string, int(*)(const char*, WS_Info)> Map_WS_API;
extern unordered_map<string, unordered_set<string>> Map_AddFriend;


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
    for (it = Map_WS_User.begin(); it != Map_WS_User.end();)
    {

        if (server->get_con_from_hdl(it->second.hdl) == server->get_con_from_hdl(hdl))
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


int WS_AddFriend(const char* JsonData, WS_Info Info)
{
    Document DocReceive;
    Document DocSend;
    DocSend.SetObject();
    int nRow;
    DocReceive.Parse(JsonData);
    if (DocReceive.HasMember("src") && DocReceive.HasMember("dst") && DocReceive.HasMember("status"))
    {
        Value &vsrc = DocReceive["src"];
        Value &vdst = DocReceive["dst"];
        Value &vstatus = DocReceive["status"];
        string src = vsrc.GetString();
        string dst = vdst.GetString();
        string status = vstatus.GetString();
        unordered_map<string, WS_Info>::iterator it;
        it = Map_WS_User.find(src);
        if (it != Map_WS_User.end())  //检查是否登陆
        {
            if (it->second.server->get_con_from_hdl(it->second.hdl) == Info.server->get_con_from_hdl(Info.hdl))
            {
                string SQL = R"(select * from user where id=")" + dst + R"(";)";
                vector<vector<string>> Result(10);
                Conn.ExecSQL(SQL.c_str(), Result, nRow);
                if (nRow == 0)//无此用户
                {
                    DocSend.AddMember("command", "add_friend_return", DocSend.GetAllocator());
                    DocSend.AddMember("status", "fail", DocSend.GetAllocator());
                    DocSend.AddMember("datail", "wrong friend_id", DocSend.GetAllocator());
                }
                else
                {
                    SQL = R"(select * from friendlist where id=")" + src + R"(" and friend_id=")" + dst + R"(";)";
                    Conn.ExecSQL(SQL.c_str(), Result, nRow);
                    if (nRow == 0)  //好友列表中无该好友   应把消息转发到dst处
                    {
                        Document DocFwd;
                        DocFwd.SetObject();
                        DocFwd.Parse(JsonData);
                        DocFwd.AddMember("message", "confirm_one_way", DocFwd.GetAllocator());
                        StringBuffer buffer;
                        PrettyWriter<StringBuffer> writer(buffer);
                        DocFwd.Accept(writer);
                        string Confirm = buffer.GetString();
                        Packet Packet_send;
                        memset(&Packet_send, 0, BUF_SIZE + 4);
                        Packet_send.Length = Confirm.size();
                        memcpy(Packet_send.Data, Confirm.c_str(), Confirm.size());
                        Mtx_Lock(mtx_Packet);
                        Packet_Queue.push(Packet_send);
                        Mtx_Unlock(mtx_Packet);

                        unordered_map<string, unordered_set<string>>::iterator it;
                        Mtx_Lock(mtx_Map_AddFriend);
                        it = Map_AddFriend.find(src);
                        if (it == Map_AddFriend.end())
                        {
                            unordered_set<string> Set_dst;
                            Set_dst.insert(dst);
                            Map_AddFriend.insert(pair<string, unordered_set<string>>(src, Set_dst));
                        }
                        else
                        {
                            it->second.insert(dst);
                        }
                        Mtx_Unlock(mtx_Map_AddFriend);
                        DocSend.AddMember("command", "add_friend_return", DocSend.GetAllocator());
                        DocSend.AddMember("status", "success", DocSend.GetAllocator());
                    }
                    else  //已经有该好友
                    {
                        DocSend.AddMember("command", "add_friend_return", DocSend.GetAllocator());
                        DocSend.AddMember("status", "fail", DocSend.GetAllocator());
                        DocSend.AddMember("datail", "already have this friend", DocSend.GetAllocator());
                    }
                }
            }
            else  //用户与socket不匹配
            {
                DocSend.AddMember("command", "add_friend_return", DocSend.GetAllocator());
                DocSend.AddMember("status", "fail", DocSend.GetAllocator());
                DocSend.AddMember("datail", "not logged in or using different socket", DocSend.GetAllocator());
            }
        }
        else  //未登录
        {
            DocSend.AddMember("command", "add_friend_return", DocSend.GetAllocator());
            DocSend.AddMember("status", "fail", DocSend.GetAllocator());
            DocSend.AddMember("datail", "not logged in", DocSend.GetAllocator());
        }
    }
    else //json包错误
    {
        DocSend.AddMember("command", "add_friend_return", DocSend.GetAllocator());
        DocSend.AddMember("status", "fail", DocSend.GetAllocator());
        DocSend.AddMember("datail", "wrong Json", DocSend.GetAllocator());
    }
    StringBuffer buffer;
    PrettyWriter<StringBuffer> writer(buffer);
    DocSend.Accept(writer);
    string JsonSend = buffer.GetString();
    Info.server->send(Info.hdl, JsonSend.c_str(), websocketpp::frame::opcode::text);
    return 0;
}



int WS_AddFriendConfirm(const char* JsonData, WS_Info Info)  //此函数中dst为登陆用户 src为好友
{
    Document DocReceive;
    Document DocSend;
    DocSend.SetObject();
    int nRow;
    DocReceive.Parse(JsonData);
    if (DocReceive.HasMember("src") && DocReceive.HasMember("dst") && DocReceive.HasMember("status"))
    {
        Value &vsrc = DocReceive["src"];
        Value &vdst = DocReceive["dst"];
        Value &vstatus = DocReceive["status"];
        Value &vmessage = DocReceive["message"];
        string src = vsrc.GetString();
        string dst = vdst.GetString();
        string status = vstatus.GetString();
        string message = vmessage.GetString();
        unordered_map<string, WS_Info>::iterator it;
        it = Map_WS_User.find(dst);  //此时的dst才是登陆的用户  
        if (it != Map_WS_User.end())  //检查是否登陆
        {
            if (it->second.server->get_con_from_hdl(it->second.hdl) == Info.server->get_con_from_hdl(Info.hdl))
            {
                string SQL = R"(select * from user where id=")" + src + R"(";)";
                vector<vector<string>> Result(10);
                Conn.ExecSQL(SQL.c_str(), Result, nRow);
                if (nRow == 0)//无此用户
                {
                    DocSend.AddMember("command", "add_friend_confirm_return", DocSend.GetAllocator());
                    DocSend.AddMember("status", "fail", DocSend.GetAllocator());
                    DocSend.AddMember("datail", "wrong src", DocSend.GetAllocator());
                }
                else
                {
                    if (message == "confirm_two_way")
                    {
                        SQL = R"(select * from friendlist where id=")" + dst + R"(" and friend_id=")" + src + R"(";)";
                        Conn.ExecSQL(SQL.c_str(), Result, nRow);
                        if (nRow == 0)  //好友列表中无该好友    添加好友
                        {
                            unordered_map<string, unordered_set<string>>::iterator it_map;
                            Mtx_Lock(mtx_Map_AddFriend);
                            it_map = Map_AddFriend.find(src);
                            if (it_map != Map_AddFriend.end())
                            {
                                unordered_set<string>::iterator it_set;
                                it_set = it_map->second.find(dst);
                                if (it_set != it_map->second.end())
                                {
                                    string SQL = R"(insert into friendlist values(")" + src + R"(",")" + dst + R"(",")" + status + R"(");)";
                                    Conn.ExecSQL(SQL.c_str(), Result, nRow);
                                    SQL = R"(insert into friendlist values(")" + dst + R"(",")" + src + R"(",")" + status + R"(");)";
                                    Conn.ExecSQL(SQL.c_str(), Result, nRow);
                                    DocSend.AddMember("command", "add_friend_confirm_return", DocSend.GetAllocator());
                                    DocSend.AddMember("status", "success", DocSend.GetAllocator());
                                }
                                else//伪造的confirm包
                                {

                                }

                            }
                            else  //伪造的confirm包
                            {

                            }
                            Mtx_Unlock(mtx_Map_AddFriend);
                        }
                        else  //已经有该好友
                        {
                            DocSend.AddMember("command", "add_friend_confirm_return", DocSend.GetAllocator());
                            DocSend.AddMember("status", "fail", DocSend.GetAllocator());
                            DocSend.AddMember("datail", "already have this friend", DocSend.GetAllocator());
                        }
                    }
                    else  //不添加好友
                    {

                    }

                }
            }
            else  //用户与socket不匹配
            {
                DocSend.AddMember("command", "add_friend_confirm_return", DocSend.GetAllocator());
                DocSend.AddMember("status", "fail", DocSend.GetAllocator());
                DocSend.AddMember("datail", "not logged in or using different socket", DocSend.GetAllocator());
            }
        }
        else  //未登录
        {
            DocSend.AddMember("command", "add_friend_confirm_return", DocSend.GetAllocator());
            DocSend.AddMember("status", "fail", DocSend.GetAllocator());
            DocSend.AddMember("datail", "not logged in", DocSend.GetAllocator());
        }
    }
    else //json包错误
    {
        DocSend.AddMember("command", "add_friend_confirm_return", DocSend.GetAllocator());
        DocSend.AddMember("status", "fail", DocSend.GetAllocator());
        DocSend.AddMember("datail", "wrong Json", DocSend.GetAllocator());
    }
    StringBuffer buffer;
    PrettyWriter<StringBuffer> writer(buffer);
    DocSend.Accept(writer);
    string JsonSend = buffer.GetString();
    Info.server->send(Info.hdl, JsonSend.c_str(), websocketpp::frame::opcode::text);
    return 0;

}
