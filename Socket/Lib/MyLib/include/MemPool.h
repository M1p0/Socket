#pragma once
#include <list>
#include <mutex>
#include <iostream>
using namespace std;
struct  Buffer
{
    bool bUsed;
    size_t uBufferSize;
    void* pBufferAddress;
    Buffer() :bUsed(false), uBufferSize(0), pBufferAddress(nullptr) {};
};

class MemPool
{
public:
    MemPool();
    ~MemPool();
    void* Allocate(size_t uSize);
    void Deallocate(void *p);
    void SetBlockSize(size_t uSize);
    size_t GetBlockSize();
    size_t Capacity();
private:
    mutex mtx;
    list<Buffer> BufferList;
    size_t uBlockSize;

};



template<class T> inline void Destroy(T *p, MemPool &mempool)
{
    if (p)
    {
        p->~T();
        mempool.Deallocate(p);
    }
};

inline void* operator new(size_t uSize, MemPool &mempool)
{
    return mempool.Allocate(uSize);
};

inline void operator delete(void *p, MemPool &mempool)
{
    Destroy(p, mempool);
}

/*
int main()
{
    MemPool pool;
    int *array = new(pool) int[1024];
    Destroy(array, pool);
}
*/