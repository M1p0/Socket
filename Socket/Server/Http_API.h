#pragma once
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/http.h>
#include <evhttp.h>
#include <string>
using namespace std;
int Logon_API(const char* JsonData, char* JsonSend);
int Login_API(const char* JsonData, char* JsonSend);
int ListFriend_API(const char* JsonData, char* JsonSend);