#pragma once
int Logon(const char* JsonData, SOCKET sClient);
int Login(const char* JsonData, SOCKET sClient);
int Logout(const char* JsonData, SOCKET sClient);
int AddFriend(const char* JsonData, SOCKET sClient);
int ListFriend(const char* JsonData, SOCKET sClient);
int SendMessage(const char* JsonData, SOCKET sClient);
int AddFriendConfirm(const char* JsonData, SOCKET sClient);