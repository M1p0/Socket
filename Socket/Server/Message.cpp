#include <MSocket.h>
#include "Message.h"
#include "MJson.h"
#include <MDatabase.h>
#include <unordered_map>
#include <unordered_set>
#include <MyEvent.h>
#include <string.h>
#include <iostream>
#include <string>
#include <queue>
#include <vector>
#include <Public.h>
using namespace std;
using namespace rapidjson;


extern mutex mtx_CIP;
extern mutex mtx_CSocket;
extern mutex mtx_Packet;
extern mutex mtx_Map_User;
extern queue <Packet> Packet_Queue;
extern MSocket Sock;
extern MDatabase Conn;
extern unordered_map<string, SOCKET> Map_User;

unordered_map<string, unordered_set<string>> Map_AddFriend;
mutex mtx_Map_AddFriend;

int AddFriendConfirm(const char* JsonData, SOCKET sClient)  //此函数中dst为登陆用户 src为好友
{
    bool confirmed = false;
    string temp_src = "0";
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
        temp_src = src;
        string dst = vdst.GetString();
        string status = vstatus.GetString();
        string message = vmessage.GetString();
        unordered_map<string, SOCKET>::iterator it;
        Mtx_Lock(mtx_Map_User);
        it = Map_User.find(dst);  //此时的dst才是登陆的用户  
        if (it != Map_User.end())  //检查是否登陆
        {
            if (it->second == sClient)
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
                    if (message=="confirm_two_way")
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
                                    confirmed = true;
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
                Mtx_Unlock(mtx_Map_User);
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
    Packet Packet_Send;
    memset(Packet_Send.Data, 0, BUF_SIZE);
    Packet_Send.Length = JsonSend.size();
    memcpy(Packet_Send.Data, JsonSend.c_str(), JsonSend.size());
    Sock.Send(sClient, (char*)&Packet_Send, JsonSend.size() + 4);

    if (confirmed)
    {
        unordered_map<string, SOCKET>::iterator it;
        Mtx_Lock(mtx_Map_User);
        it = Map_User.find(temp_src);
        if (it!=Map_User.end())
        {
            Sock.Send(it->second, (char*)&Packet_Send, JsonSend.size() + 4);
        }
        Mtx_Unlock(mtx_Map_User);
    }
    return 0;

}


int AddFriend(const char* JsonData, SOCKET sClient)
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
        unordered_map<string, SOCKET>::iterator it;
        Mtx_Lock(mtx_Map_User);
        it = Map_User.find(src);
        if (it != Map_User.end())  //检查是否登陆
        {
            if (it->second == sClient)
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
                        if (it==Map_AddFriend.end())
                        {
                            unordered_set<string> Set_dst;
                            Set_dst.insert(dst);
                            Map_AddFriend.insert(pair<string, unordered_set<string>>(src,Set_dst));
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
            Mtx_Unlock(mtx_Map_User);
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
    Packet Packet_Send;
    memset(Packet_Send.Data, 0, BUF_SIZE);
    Packet_Send.Length = JsonSend.size();
    memcpy(Packet_Send.Data, JsonSend.c_str(), JsonSend.size());
    Sock.Send(sClient, (char*)&Packet_Send, JsonSend.size() + 4);
    return 0;

}


int PushOfflineMessage(const char* id)
{
    MSleep(1, "s");
    int nRow = 0;
    string ID = id;
    string SQL = R"(select * from offline_message where dst=")" + ID + R"(";)";
    vector<vector<string>> Result(1024);
    Conn.ExecSQL(SQL.c_str(), Result, nRow);

    if (nRow == 0)
    {
        return 0;
    }
    else
    {
        for (int i = 0; i < nRow; i++)
        {
            Document DocSend;
            DocSend.SetObject();
            Value vsrc;
            Value vdst;
            Value vmessage;
            string src = Result[i][0];
            string dst = Result[i][1];
            string message = Result[i][2];
            vsrc.SetString(src.c_str(), src.size(), DocSend.GetAllocator());
            vdst.SetString(dst.c_str(), dst.size(), DocSend.GetAllocator());
            vmessage.SetString(message.c_str(), message.size(), DocSend.GetAllocator());
            DocSend.AddMember("command", "send_message", DocSend.GetAllocator());
            DocSend.AddMember("src", vsrc, DocSend.GetAllocator());
            DocSend.AddMember("dst", vdst, DocSend.GetAllocator());
            DocSend.AddMember("message", vmessage, DocSend.GetAllocator());

            StringBuffer buffer;
            PrettyWriter<StringBuffer> writer(buffer);
            DocSend.Accept(writer);
            string JsonSend = buffer.GetString();
            Packet Packet_send;
            memset(&Packet_send.Data, 0, BUF_SIZE);
            Packet_send.Length = JsonSend.size();
            memcpy(Packet_send.Data, JsonSend.c_str(), JsonSend.size());
            Mtx_Lock(mtx_Packet);
            Packet_Queue.push(Packet_send);
            Mtx_Unlock(mtx_Packet);
            SQL = R"(delete from offline_message where message=")" + message + R"(")" + R"( and dst=")" + dst + R"(";)";  //验证时间
            Conn.ExecSQL(SQL.c_str(), Result, nRow);
        }
    }
    return 0;
}


int Logon(const char* JsonData, SOCKET sClient)
{

    Document DocReceive;
    Document DocSend;
    DocSend.SetObject();
    int nRow;
    DocReceive.Parse(JsonData);

    if (DocReceive.HasMember("username") && DocReceive.HasMember("password"))
    {
        Value &vusername = DocReceive["username"];
        Value &vpassword = DocReceive["password"];
        string username = vusername.GetString();
        string password = vpassword.GetString();
        string SQL = R"(begin;)";
        SQL = SQL + R"(insert into user(username,password) values(")" + username + R"(",")" + password + R"(");)";
        SQL = SQL + R"(select id from user order by ID DESC limit 1;)";
        SQL = SQL + R"(commit;)";
        vector<vector<string>> Result(1);
        Conn.ExecSQL(SQL.c_str(), Result, nRow);
        Value ID(Result[0][0].c_str(), DocSend.GetAllocator());
        DocSend.AddMember("command", "logon_return", DocSend.GetAllocator());
        DocSend.AddMember("status", "success", DocSend.GetAllocator());
        DocSend.AddMember("id", ID, DocSend.GetAllocator());
    }
    else
    {
        DocSend.AddMember("command", "logon_return", DocSend.GetAllocator());
        DocSend.AddMember("status", "fail", DocSend.GetAllocator());
        DocSend.AddMember("detail", "wrong Json", DocSend.GetAllocator());
    }

    StringBuffer buffer;
    PrettyWriter<StringBuffer> writer(buffer);
    DocSend.Accept(writer);
    string JsonSend = buffer.GetString();
    Packet Packet_Send;
    memset(Packet_Send.Data, 0, BUF_SIZE);
    Packet_Send.Length = JsonSend.size();
    memcpy(Packet_Send.Data, JsonSend.c_str(), JsonSend.size());
    Sock.Send(sClient, (char*)&Packet_Send, JsonSend.size() + 4);
    return 0;


}


int Login(const char* JsonData, SOCKET sClient)
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
        if (nRow == 0)  //账号密码错误
        {
            DocSend.AddMember("command", "login_return", DocSend.GetAllocator());
            DocSend.AddMember("status", "fail", DocSend.GetAllocator());
            DocSend.AddMember("detail", "wrong id or password", DocSend.GetAllocator());
        }
        else
        {
            Value vUsername(Result[0][0].c_str(), DocSend.GetAllocator());
            DocSend.AddMember("command", "login_return", DocSend.GetAllocator());
            DocSend.AddMember("status", "success", DocSend.GetAllocator());
            DocSend.AddMember("username", vUsername, DocSend.GetAllocator());
            Mtx_Lock(mtx_Map_User);
            Map_User.insert(pair<string, SOCKET>(id, sClient));
            Mtx_Unlock(mtx_Map_User);
            MSleep(10, "ms");

        }
    }
    else
    {
        DocSend.AddMember("command", "login_return", DocSend.GetAllocator());
        DocSend.AddMember("status", "fail", DocSend.GetAllocator());
        DocSend.AddMember("detail", "wrong Json", DocSend.GetAllocator());
    }
    StringBuffer buffer;
    PrettyWriter<StringBuffer> writer(buffer);
    DocSend.Accept(writer);
    string JsonSend = buffer.GetString();
    Packet Packet_Send;
    memset(Packet_Send.Data, 0, BUF_SIZE);
    Packet_Send.Length = JsonSend.size();
    memcpy(Packet_Send.Data, JsonSend.c_str(), JsonSend.size());
    Sock.Send(sClient, (char*)&Packet_Send, JsonSend.size() + 4);
    if (id != "0")
    {
        PushOfflineMessage(id.c_str());
    }
    return 0;

}



int Logout(const char* JsonData, SOCKET sClient)
{
    Document DocReceive;
    DocReceive.Parse(JsonData);
    if (DocReceive.HasMember("id"))
    {
        Value &vid = DocReceive["id"];
        string id = vid.GetString();
        unordered_map<string, SOCKET>::iterator it;
        it = Map_User.find(id);
        if (it->second == sClient)
        {
            Mtx_Lock(mtx_Map_User);
            Map_User.erase(it++);
            Mtx_Unlock(mtx_Map_User);
            Sock.Close(sClient);
            return 0;
        }
        else   //理论上不用管
        {
            return -1;
        }
    }
    else
    {
        return -1;
    }

}




int ListFriend(const char* JsonData, SOCKET sClient)
{
    Document DocReceive;
    Document DocSend;
    DocSend.SetObject();
    int nRow;
    DocReceive.Parse(JsonData);
    if (DocReceive.HasMember("id"))
    {
        Value &vid = DocReceive["id"];
        string id = vid.GetString();
        unordered_map<string, SOCKET>::iterator it;
        it = Map_User.find(id);
        if (it != Map_User.end())  //检查是否登陆
        {
            if (it->second == sClient)  //匹配socket
            {
                Value vid;
                vid.SetString(id.c_str(), id.size(), DocSend.GetAllocator());
                string SQL = R"(select * from friendlist where id=")" + id + R"(";)";
                vector<vector<string>> Result(1024);  //最多1024
                Conn.ExecSQL(SQL.c_str(), Result, nRow);
                if (nRow != 0)
                {
                    DocSend.AddMember("command", "list_friend_return", DocSend.GetAllocator());
                    DocSend.AddMember("id", vid, DocSend.GetAllocator());
                    Value Array_Friends(kArrayType);
                    for (int i = 0; i < nRow; i++)
                    {
                        int num = 0;
                        string Friend_id = Result[i][1];
                        SQL = R"(select username from user where id=")" + Friend_id + R"(";)";
                        vector<vector<string>> Result_Username(1);
                        Conn.ExecSQL(SQL.c_str(), Result_Username, num);
                        if (num!=0)
                        {
                            Value Temp(kObjectType);   //临时的object对象   存储friend信息
                            Value vFriend_id;
                            Value vStatus;
                            Value vUsername;
                            vUsername.SetString(Result_Username[0][0].c_str(), Result_Username[0][0].size(), DocSend.GetAllocator());
                            vFriend_id.SetString(Result[i][1].c_str(), Result[i][1].size(), DocSend.GetAllocator());
                            vStatus.SetString(Result[i][2].c_str(), Result[i][2].size(), DocSend.GetAllocator());
                            Temp.AddMember("id", vFriend_id, DocSend.GetAllocator());
                            Temp.AddMember("username", vUsername, DocSend.GetAllocator());
                            Temp.AddMember("status", vStatus, DocSend.GetAllocator());
                            Array_Friends.PushBack(Temp, DocSend.GetAllocator());
                        }
                        else  //清除错误条目
                        {
                            SQL= R"(delete from friendlist where friend_id=")" + Friend_id + R"(";)";
                            Conn.ExecSQL(SQL.c_str(), Result_Username, num);  //无返回集
                        }
                    }
                    DocSend.AddMember("friends", Array_Friends, DocSend.GetAllocator());
                }
                else  //无此用户
                {
                    DocSend.AddMember("command", "list_friend_return", DocSend.GetAllocator());
                    DocSend.AddMember("status", "fail", DocSend.GetAllocator());
                    DocSend.AddMember("datail", "wrong id", DocSend.GetAllocator());
                }
            }
            else
            {
                DocSend.AddMember("command", "list_friend_return", DocSend.GetAllocator());
                DocSend.AddMember("status", "fail", DocSend.GetAllocator());
                DocSend.AddMember("datail", "socket not match", DocSend.GetAllocator());
            }

        }
        else
        {
            DocSend.AddMember("command", "list_friend_return", DocSend.GetAllocator());
            DocSend.AddMember("status", "fail", DocSend.GetAllocator());
            DocSend.AddMember("datail", "not logged in", DocSend.GetAllocator());
        }
    }
    else   //json包错误
    {
        DocSend.AddMember("command", "list_friend_return", DocSend.GetAllocator());
        DocSend.AddMember("status", "fail", DocSend.GetAllocator());
        DocSend.AddMember("datail", "wrong Json", DocSend.GetAllocator());
    }
    StringBuffer buffer;
    PrettyWriter<StringBuffer> writer(buffer);
    DocSend.Accept(writer);
    string Data = buffer.GetString();
    Packet Packet_Send;
    memset(&Packet_Send, 0, BUF_SIZE + 4);
    Packet_Send.Length = Data.size();
    memcpy(Packet_Send.Data, Data.c_str(), Data.size());
    Sock.Send(sClient, (char*)&Packet_Send, Data.size() + 4);
    return 0;
}


int SendMessage(const char* JsonData, SOCKET sClient)
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


        unordered_map<string, SOCKET>::iterator it;
        it = Map_User.find(src);
        if (it != Map_User.end())  //检查是否登陆
        {
            string SQL = R"(select * from friendlist where id=")" + src + R"(")" + R"(and friend_id=")" + dst + R"(";)";
            vector<vector<string>> Result(1);
            Conn.ExecSQL(SQL.c_str(), Result, nRow);

            if (nRow == 0)  //无此好友关系
            {
                DocSend.AddMember("command", "send_message_return", DocSend.GetAllocator());
                DocSend.AddMember("status", "fail", DocSend.GetAllocator());
                DocSend.AddMember("datail", "dst not in friend_list", DocSend.GetAllocator());
            }
            else
            {
                DocSend.AddMember("command", "send_message_return", DocSend.GetAllocator());
                DocSend.AddMember("status", "success", DocSend.GetAllocator());
                Packet Packet_send;
                memset(&Packet_send, 0, BUF_SIZE + 4);
                Packet_send.Length = strlen(JsonData);
                memcpy(Packet_send.Data, JsonData, strlen(JsonData));
                Mtx_Lock(mtx_Packet);
                Packet_Queue.push(Packet_send);
                Mtx_Unlock(mtx_Packet);
            }
        }
        else   //socket错误
        {
            DocSend.AddMember("command", "send_message_return", DocSend.GetAllocator());
            DocSend.AddMember("status", "fail", DocSend.GetAllocator());
            DocSend.AddMember("datail", "not logged in or using different socket", DocSend.GetAllocator());
        }
    }
    else  //json错误
    {
        DocSend.AddMember("command", "send_message_return", DocSend.GetAllocator());
        DocSend.AddMember("status", "fail", DocSend.GetAllocator());
        DocSend.AddMember("datail", "wrong Json", DocSend.GetAllocator());
    }


    StringBuffer buffer;
    PrettyWriter<StringBuffer> writer(buffer);
    DocSend.Accept(writer);
    string Data = buffer.GetString();
    Packet Packet_Send;
    memset(&Packet_Send, 0, BUF_SIZE + 4);
    Packet_Send.Length = Data.size();
    memcpy(Packet_Send.Data, Data.c_str(), Data.size());
    Sock.Send(sClient, (char*)&Packet_Send, Data.size() + 4);
    return 0;
}

