#include "Http_Server.h"
#include <unordered_map>
#include <iostream>
#include <string>
#include <CFileIO.h>
#include "MJson.h"
#include "Utils.h"
#include "Logic_API.h"

using namespace std;
unordered_map<string, int(*)(const char*, char*)> Map_Logic_API;
unordered_map<string, string> Map_Content_Type;
extern CFileIO File;
#define BUFFER_SIZE 1024*1024



bool startHttpServer(const char *ip, int port, void(*cb)(struct evhttp_request *, void *), void *arg)
{
    //创建event_base和evhttp
    event_base* base = event_base_new();
    evhttp* http_server = evhttp_new(base);

    if (!http_server)
    {
        return false;
    }
    //绑定到指定地址上
    int ret = evhttp_bind_socket(http_server, ip, port & 0xFFFF);
    if (ret != 0)
    {
        return false;
    }
    //设置事件处理函数
    evhttp_set_gencb(http_server, cb, arg);
    //启动事件循环，当有http请求的时候会调用指定的回调
    cout << "Http Server Running..." << endl;
    event_base_dispatch(base);
    evhttp_free(http_server);
    return true;
}


void MyHttpServerHandler(struct evhttp_request* req, void* arg)
{
    struct evkeyvalq *Req_Header;
    Req_Header = evhttp_request_get_input_headers(req);  //request头
    evbuffer* buf = evbuffer_new();
    char* buffer = new char[BUFFER_SIZE];
    char local[4096];
    memset(buffer, 0, BUFFER_SIZE);
    memset(local, 0, 4096);
    local[0] = '.';
    const char* uri = (char*)evhttp_request_get_uri(req);   //请求的uri
    cout << "uri:" << uri << endl;

    if (_stricmp(uri, "/api") == 0)
    {
        int ev_input_data_length = evbuffer_get_length(req->input_buffer);
        unsigned char* ev_input_data = evbuffer_pullup(req->input_buffer, ev_input_data_length);
        string type;
        for (evkeyval* header = Req_Header->tqh_first; header; header = header->next.tqe_next)
        {
            if (_stricmp(header->key, "content-type") == 0)
            {
                type = header->value;
                break;
            }
        }
        if (_stricmp(type.c_str(), "application/x-www-form-urlencoded") == 0)  //post
        {
            char input_data[BUF_SIZE];
            memset(input_data, 0, BUF_SIZE);
            memcpy(input_data, ev_input_data, ev_input_data_length);

            Document document;
            Document DocSend;
            DocSend.SetObject();
            document.Parse(input_data);
            if (document.IsObject())
            {
                if (document.HasMember("command"))
                {
                    Value &value1 = document["command"];
                    string command = value1.GetString();
                    unordered_map<string, int(*)(const char*, char*)>::iterator it;  //调用已有api
                    it = Map_Logic_API.find(command);
                    if (it != Map_Logic_API.end())
                    {
                        char JsonSend[BUF_SIZE];
                        memset(JsonSend, 0, BUF_SIZE);
                        it->second(input_data, JsonSend);
                        cout << "Received:" << input_data << endl;
                        int64_t uLength = strlen(JsonSend);
                        char szLength[32];
                        memset(szLength, 0, 32);
                        _i64toa(uLength, szLength, 10);
                        evbuffer_expand(buf, uLength);   //增大缓冲区
                        evhttp_add_header(req->output_headers, "Content-Length", szLength);
                        evbuffer_add(buf, JsonSend, uLength);
                    }
                    else//wrong command
                    {
                        DocSend.AddMember("command", "return", DocSend.GetAllocator());
                        DocSend.AddMember("status", "fail", DocSend.GetAllocator());
                        DocSend.AddMember("detail", "wrong command", DocSend.GetAllocator());
                        StringBuffer buffer;
                        PrettyWriter<StringBuffer> writer(buffer);
                        DocSend.Accept(writer);
                        string JsonSend = buffer.GetString();
                        evbuffer_add(buf, JsonSend.c_str(), JsonSend.size());
                    }
                }
                else//no command in Json
                {
                    DocSend.AddMember("command", "return", DocSend.GetAllocator());
                    DocSend.AddMember("status", "fail", DocSend.GetAllocator());
                    DocSend.AddMember("detail", "no command in Json", DocSend.GetAllocator());
                    StringBuffer buffer;
                    PrettyWriter<StringBuffer> writer(buffer);
                    DocSend.Accept(writer);
                    string JsonSend = buffer.GetString();
                    evbuffer_add(buf, JsonSend.c_str(), JsonSend.size());
                }
            }
            else  //not an Json Object
            {
                DocSend.AddMember("command", "return", DocSend.GetAllocator());
                DocSend.AddMember("status", "fail", DocSend.GetAllocator());
                DocSend.AddMember("detail", "not an Json Object", DocSend.GetAllocator());
                StringBuffer buffer;
                PrettyWriter<StringBuffer> writer(buffer);
                DocSend.Accept(writer);
                string JsonSend = buffer.GetString();
                evbuffer_add(buf, JsonSend.c_str(), JsonSend.size());
            }

        }
        else //not post
        {
            return;
        }

    }
    else  //其他页面
    {
        strcat(local, uri);
        size_t Pos = 0;//找出第一个"?"
        FindFirst(uri, "?", Pos);
        if (Pos != 0)
        {
            local[Pos + 1] = '\0';
        }
        File.Read(local, buffer, 0, BUFFER_SIZE);
        int64_t uLength;
        File.GetSize(local, &uLength);
        char szLength[32];
        _i64toa(uLength, szLength, 10);
        evbuffer_expand(buf, uLength);   //增大缓冲区

        string type;
        for (evkeyval* header = Req_Header->tqh_first; header; header = header->next.tqe_next)
        {
            if (_stricmp(header->key, "content-type") == 0)
            {
                type = header->value;
            }
        }
        //回复给客户端
        evhttp_add_header(req->output_headers, "Content-Length", szLength);
        //添加content-type
        char Temp[128];
        memset(Temp, 0, 128);
        GetExtension(local, Temp);
        string Extension = Temp;
        string Content_Type;
        unordered_map<string, string>::iterator it;
        it = Map_Content_Type.find(Extension);
        if (it != Map_Content_Type.end())
        {
            Content_Type = it->second;
        }
        else
        {
            Content_Type = "text/html";
        }
        evhttp_add_header(req->output_headers, "Content-Type", Content_Type.c_str());
        evbuffer_add(buf, buffer, uLength);
    }


    evhttp_send_reply(req, HTTP_OK, "OK", buf);
    delete[] buffer;
    evbuffer_free(buf);
}

void Create_Type_Map()
{
    char* buffer = new char[1024 * 1024];
    memset(buffer, 0, 1024 * 1024);
    File.Read("./content-type.json", buffer, 0, 1024 * 1024);
    Document document;
    document.Parse(buffer);
    delete[] buffer;
    if (document.IsObject())
    {
        for (Value::ConstMemberIterator it = document.MemberBegin(); it != document.MemberEnd(); it++)
        {
            Map_Content_Type.insert(pair<string, string>(it->name.GetString(), it->value.GetString()));
        }
        //for (unordered_map<string, string>::iterator it=Map_Content_Type.begin();it!=Map_Content_Type.end();it++)
        //{
        //    cout << it->first << it->second << endl;
        //}
    }
    else
    {
        cout << R"(Wrong Json!Check "./content-type.json")" << endl;
        return;
    }
}