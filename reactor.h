#include"Epoll.h"
#include"socketconnect.h"

class reactor{
    Epoll *epoll;
    socketconnect *conn;
    threadpool<Handler> *pool;
    public:
    reactor(int port);
    ~reactor();
    void run();
};