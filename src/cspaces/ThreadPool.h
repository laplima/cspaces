// #include <pthread.h>
// #include <tao/corba.h>
#include <colibry/ORBManager.h>
#include <thread>
#include <list>

//
// Thread pool
//
// TAO does support thread pool semantics for the ORB_CTRL_MODEL POA policy.
// However, TAO requires that the application programmer create all the
// threads in the thread pool, and each of these threads must call orb->run(),
// which is how the application-level threads become part of the thread pool.

class ThreadPool {
public:
    ThreadPool(colibry::ORBManager& om, unsigned short count);
    virtual ~ThreadPool();
    void join();
private:
    std::list<std::thread> workers_;
};
