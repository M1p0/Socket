#include "Http_API.h"
#include "MJson.h"
#include <MSocket.h>
#include <iostream>
#include <string>
#include <vector>
#include <MDatabase.h>
using namespace std;
extern MDatabase Conn;

int Logon_API(const char* JsonData, char* JsonSend)
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
        DocSend.SetObject();
        DocSend.AddMember("command", "logon_return", DocSend.GetAllocator());
        DocSend.AddMember("status", "success", DocSend.GetAllocator());
        DocSend.AddMember("id", ID, DocSend.GetAllocator());
        DocSend.AddMember("username", vusername, DocSend.GetAllocator());
        StringBuffer buffer;
        PrettyWriter<StringBuffer> writer(buffer);
        DocSend.Accept(writer);
        memcpy(JsonSend, buffer.GetString(), buffer.GetSize());
        return 0;
    }
    else
    {
        DocSend.AddMember("command", "logon_return", DocSend.GetAllocator());
        DocSend.AddMember("status", "fail", DocSend.GetAllocator());
        DocSend.AddMember("detail", "wrong json", DocSend.GetAllocator());
        StringBuffer buffer;
        PrettyWriter<StringBuffer> writer(buffer);
        DocSend.Accept(writer);
        memcpy(JsonSend, buffer.GetString(), buffer.GetSize());
        return -1;
    }

}

int Login_API(const char* JsonData, char* JsonSend)
{
    Document DocReceive;
    Document DocSend;
    DocSend.SetObject();
    int nRow;
    DocReceive.Parse(JsonData);
    if (DocReceive.HasMember("id") && DocReceive.HasMember("password"))
    {
        Value &vid = DocReceive["id"];
        Value &vpassword = DocReceive["password"];
        string id = vid.GetString();
        string password = vpassword.GetString();
        string SQL = R"(select username from user where id=")" + id + R"(")" + R"(and password=")" + password + R"(";)";
        vector<vector<string>> Result(1);
        Conn.ExecSQL(SQL.c_str(), Result, nRow);
        if (nRow==0)   //wrong password
        {
            DocSend.AddMember("command", "login_return", DocSend.GetAllocator());
            DocSend.AddMember("status", "fail", DocSend.GetAllocator());
            DocSend.AddMember("detail", "wrong password or id", DocSend.GetAllocator());
        }
        else
        {
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
    memcpy(JsonSend, buffer.GetString(), buffer.GetSize());
    return 0;


}



int ListFriend_API(const char* JsonData, char* JsonSend)
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
                if (num != 0)
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
                    SQL = R"(delete from friendlist where friend_id=")" + Friend_id + R"(";)";
                    Conn.ExecSQL(SQL.c_str(), Result_Username, num);  //无返回集
                }
            }
            DocSend.AddMember("status", "success", DocSend.GetAllocator());
            DocSend.AddMember("friends", Array_Friends, DocSend.GetAllocator());
            StringBuffer buffer;
            PrettyWriter<StringBuffer> writer(buffer);
            DocSend.Accept(writer);
            memcpy(JsonSend, buffer.GetString(), buffer.GetSize());
            return 0;
        }
        else  //无此用户
        {
            DocSend.AddMember("command", "list_friend_return", DocSend.GetAllocator());
            DocSend.AddMember("status", "fail", DocSend.GetAllocator());
            DocSend.AddMember("datail", "wrong id", DocSend.GetAllocator());
            StringBuffer buffer;
            PrettyWriter<StringBuffer> writer(buffer);
            DocSend.Accept(writer);
            memcpy(JsonSend, buffer.GetString(), buffer.GetSize());
            return -1;
        }
    }
    else   //json包错误
    {
        DocSend.AddMember("command", "list_friend_return", DocSend.GetAllocator());
        DocSend.AddMember("status", "fail", DocSend.GetAllocator());
        DocSend.AddMember("datail", "wrong json", DocSend.GetAllocator());
        StringBuffer buffer;
        PrettyWriter<StringBuffer> writer(buffer);
        DocSend.Accept(writer);
        memcpy(JsonSend, buffer.GetString(), buffer.GetSize());
        return -1;
    }
}
