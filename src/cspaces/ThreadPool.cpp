#include "ThreadPool.h"

using namespace std;
using namespace colibry;

ThreadPool::ThreadPool(colibry::ORBManager& om, unsigned short count)
{
    for (unsigned short i = 0; i<count; ++i)
        workers_.emplace_back([&om]() { om.run(); });
}

ThreadPool::~ThreadPool() { join(); }

void ThreadPool::join()
{
    for (auto& t : workers_)
        t.join();
    workers_.clear();
}
