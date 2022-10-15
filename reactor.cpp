#include"reactor.h"
reactor::reactor(int port){
        conn=new socketconnect(port);
        pool=new threadpool<Handler>(20,100);
        std::cout<<"threadpool:"<<pool<<std::endl;
        epoll=new Epoll(conn->getlisten_fd(),pool);
    }
reactor::~reactor(){
    delete conn;
    delete epoll;
    delete pool;
}
void reactor::run(){
    epoll->Epoll_init();
    epoll->Epoll_listening();
}