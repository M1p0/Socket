#include "Http_Server.h"
#include <unordered_map>
#include <iostream>
#include <CFileIO.h>
#include "MJson.h"

using namespace std;

unordered_map<string, string> Map_Content_Type;
extern CFileIO File;
#define BUFFER_SIZE 1024*1024

bool startHttpServer(const char *ip, int port, void(*cb)(struct evhttp_request *, void *), void *arg)
{
    //����event_base��evhttp
    event_base* base = event_base_new();
    evhttp* http_server = evhttp_new(base);
    if (!http_server)
    {
        return false;
    }
    //�󶨵�ָ����ַ��
    int ret = evhttp_bind_socket(http_server, ip, port & 0xFFFF);
    if (ret != 0)
    {
        return false;
    }
    //�����¼�������
    evhttp_set_gencb(http_server, cb, arg);
    //�����¼�ѭ��������http�����ʱ������ָ���Ļص�
    event_base_dispatch(base);
    evhttp_free(http_server);
    return true;
}
void MyHttpServerHandler(struct evhttp_request* req, void* arg)
{
    struct evkeyvalq *Req_Header;
    Req_Header = evhttp_request_get_input_headers(req);  //requestͷ
    evbuffer* buf = evbuffer_new();
    char* buffer = new char[BUFFER_SIZE];
    char local[4096];
    memset(buffer, 0, BUFFER_SIZE);
    memset(local, 0, 4096);
    local[0] = '.';
    const char* uri = (char*)evhttp_request_get_uri(req);   //�����uri
    cout << "uri:" << uri << endl;
    strcat(local, uri);
    File.Read(local, buffer, 0, BUFFER_SIZE);
    int64_t uLength;
    File.GetSize(local, &uLength);
    char szLength[32];
    _i64toa(uLength, szLength, 10);
    evbuffer_expand(buf, uLength);   //���󻺳���


    for (evkeyval* header = Req_Header->tqh_first; header; header = header->next.tqe_next)
    {
        if (_stricmp(header->key,"content-type")==0)
        {
        printf(" %s: %s\n", header->key, header->value);
        }

    }

    //�ظ����ͻ���
    evhttp_add_header(req->output_headers, "Content-Length", szLength);
    //evhttp_add_header(req->output_headers, "Content-Type", "application/json");
    //if (strcmp(uri,"/about.html")!=0)
    //{
    //    evhttp_add_header(req->output_headers, "Content-Type", "text/css");
    //}
    evbuffer_add(buf, buffer, uLength);
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

    }
    else
    {
        return;
    }



}