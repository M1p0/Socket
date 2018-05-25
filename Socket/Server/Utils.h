#pragma once
#include <iostream>
#include <unordered_map>
#include <MSocket.h>
#include <MDatabase.h>
#include "Message.h"
#include "Utils.h"

int InitMap();
int Log(const char* log, const char* Filename = "./.log");
void FindLast(const char* Source, const char* Target, size_t &Pos);
void FindFirst(const char* Source, const char* Target, size_t &Pos);
void GetExtension(const char* Source, char* buffer);