#pragma once

class BitMap //default 1024*4*8 states
{
public:

    BitMap();
    BitMap(unsigned int size);
    int operator[](unsigned int pos);    //左值不可修改
    unsigned int Capacity();    //实际可存容量
    unsigned int Size();    //已储存容量
    int Insert(unsigned int val);
    bool Exist(unsigned int val);
    ~BitMap();
private:
    unsigned int* Data;
    unsigned int uCapacity;    //Data数组的大小
    unsigned int uSize;         //已储存容量
};

