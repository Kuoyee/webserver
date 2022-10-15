#include <sys/epoll.h>
#include<iostream>
#include <arpa/inet.h>//////
#include"Handler.h"
#define max_fd 65536
class Epoll{
    int server_socket;
    int tree_fd;
    int myevents_count;
    Handler *handler;
    threadpool<Handler> *pool;
    struct epoll_event ev,events[256],myevents[1024];
    public:
        Epoll(int server_socket,threadpool<Handler> *pool);
        ~Epoll();
        void Epoll_init();           //创建一个红黑树结构，树的各个节点是epoll_event类型的结构体，返回值是该树根节点的索引
        void Epoll_Add(int fd);
        void Epoll_listening();
};