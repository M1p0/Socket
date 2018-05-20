#pragma once
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/http.h>
#include <evhttp.h>

bool startHttpServer(const char *ip, int port, void(*cb)(struct evhttp_request *, void *), void *arg);
void MyHttpServerHandler(struct evhttp_request* req, void* arg);