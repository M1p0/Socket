#include "WebSocket_Server.h"
#include "Utils.h"
#include <CFileIO.h>
#include "Http_API.h"

extern unordered_map<string, int(*)(const char*, SOCKET)> Map_Func;
extern unordered_map<string, int(*)(const char*, char*)> Map_Http_API;
extern unordered_map<string, int(*)(const char*, WS_Info)> Map_WS_API;
extern CFileIO File;

int Log(const char* log, const char* Filename)
{
    File.Write(Filename, log, 0, strlen(log));
    return 0;
}

int InitMap()
{
    int(*pLogon)(const char*, SOCKET) = &Logon;
    int(*pLogin)(const char*, SOCKET) = &Login;
    int(*pLogout)(const char*, SOCKET) = &Logout;
    int(*pAddFriend)(const char*, SOCKET) = &AddFriend;
    int(*pListFriend)(const char*, SOCKET) = &ListFriend;
    int(*pSendMessage)(const char*, SOCKET) = &SendMessage;
    Map_Func.insert(pair<string, int(*)(const char*, SOCKET)>("logon", pLogon));
    Map_Func.insert(pair<string, int(*)(const char*, SOCKET)>("login", pLogin));
    Map_Func.insert(pair<string, int(*)(const char*, SOCKET)>("logout", pLogout));
    Map_Func.insert(pair<string, int(*)(const char*, SOCKET)>("add_friend", pAddFriend));
    Map_Func.insert(pair<string, int(*)(const char*, SOCKET)>("list_friend", pListFriend));
    Map_Func.insert(pair<string, int(*)(const char*, SOCKET)>("send_message", pSendMessage));


    int(*pLogon_API)(const char*, char*) = &Logon_API;
    int(*pLogin_API)(const char*, char*) = &Login_API;
    int(*pAddFriend_API)(const char*, char*) = &AddFriend_API;
    int(*pListFriend_API)(const char*, char*) = &ListFriend_API;
    Map_Http_API.insert(pair<string, int(*)(const char*, char*)>("logon", pLogon_API));
    Map_Http_API.insert(pair<string, int(*)(const char*, char*)>("login", pLogin_API));
    Map_Http_API.insert(pair<string, int(*)(const char*, char*)>("add_friend", pAddFriend_API));
    Map_Http_API.insert(pair<string, int(*)(const char*, char*)>("list_friend", pListFriend_API));


    int(*pWS_Login)(const char*, WS_Info) = &WS_Login;
    int(*pWS_SendMessage)(const char*, WS_Info) = &WS_SendMessage;
    Map_WS_API.insert(pair<string, int(*)(const char*, WS_Info)>("login", pWS_Login));
    Map_WS_API.insert(pair<string, int(*)(const char*, WS_Info)>("send_message", pWS_SendMessage));

    return 0;
}



void FindLast(const char* Source, const char* Target, size_t &Pos)
{
    string Src = Source;
    Pos = Src.find_last_of(Target);
}

void FindFirst(const char* Source, const char* Target, size_t &Pos)
{
    string Src = Source;
    Pos = Src.find_first_of(Target);
    if (Pos==string::npos)
    {
        Pos = 0;
    }
}


void GetExtension(const char* Source, char* buffer)
{
    size_t Pos1 = 0;  //"."
    size_t Pos2 = 0;  //"?"
    FindLast(Source, ".", Pos1);
    FindFirst(Source, "?", Pos2);
    size_t Length = strlen(Source);
    strncat(buffer, Source + Pos1, Pos2-Pos1);
}


