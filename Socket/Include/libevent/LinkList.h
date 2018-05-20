#pragma once
#include <iostream>
using namespace std;
struct  node
{
    int Data;
    node *Next;
};

class LinkList
{
private:
    node *head;
public:
    LinkList() { head = nullptr; }  
    void Insert(int value);
    void Delete_Value(int value);
    void Delete_Pos(int pos);
    void Output();
    void Set_Loop(int start, int end);
    void Check_Loop();
    int  Length();
    
}; 