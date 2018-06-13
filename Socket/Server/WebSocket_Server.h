#pragma once
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
typedef websocketpp::server<websocketpp::config::asio> WebsocketServer;
typedef WebsocketServer::message_ptr message_ptr;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

struct WS_Info
{
    WebsocketServer *server;
    websocketpp::connection_hdl hdl;
};


void WS_Run();
int WS_Login(const char* JsonData, WS_Info Info);
int WS_SendMessage(const char* JsonData, WS_Info Info);
int WS_AddFriend(const char* JsonData, WS_Info Info);
int WS_AddFriendConfirm(const char* JsonData, WS_Info Info);

