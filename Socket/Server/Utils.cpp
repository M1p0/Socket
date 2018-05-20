#include "Utils.h"
#include <CFileIO.h>
extern unordered_map<string, int(*)(const char*, SOCKET)> Map_Func;
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
    Map_Func.insert(pair<string, int(*)(const char*, SOCKET)>("logon", pLogon));
    Map_Func.insert(pair<string, int(*)(const char*, SOCKET)>("login", pLogin));
    Map_Func.insert(pair<string, int(*)(const char*, SOCKET)>("logout", pLogout));
    Map_Func.insert(pair<string, int(*)(const char*, SOCKET)>("add_friend", pAddFriend));
    Map_Func.insert(pair<string, int(*)(const char*, SOCKET)>("list_friend", pListFriend));
    return 0;
}



