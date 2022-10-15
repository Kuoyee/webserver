#include"Epoll.h"


Epoll::Epoll(int server_socket,threadpool<Handler> *pool){
        this->server_socket=server_socket;
        this->pool=pool;
        handler=new Handler[max_fd];
};

Epoll::~Epoll(){
    delete []handler;
}

void Epoll::Epoll_init(){
        //创建一个红黑树结构，树的各个节点是epoll_event类型的结构体，返回值是该树根节点的索引
        tree_fd=epoll_create(256);
        Epoll_Add(server_socket);
}

void Epoll::Epoll_Add(int fd){
        ev.data.fd=fd;
        //设置要处理的事件类型
        ev.events=EPOLLIN|EPOLLET;
        //注册epoll事件，添加包含fd的结构体到树
        epoll_ctl(tree_fd,EPOLL_CTL_ADD,fd,&ev); 
}

void Epoll::Epoll_listening(){
        while(1){
            //nfds返回监听到有事件的个数，events传出参数，为有事件 的集合，再遍历
            int nfds=epoll_wait(tree_fd,events,256,500);

            if(nfds==-1){
                std::cout<<"epoll_wait error"<<std::endl;
                break;
            }
            for(int i=0;i<nfds;i++){
                //处理新客户端连接
                if(events[i].data.fd==server_socket){
                        char client_ip[256];
                        struct sockaddr_in client;
                        socklen_t client_size = sizeof(client);
                        int cfd = accept(server_socket, (struct sockaddr*)&client, &client_size);
                        Epoll_Add(cfd);
                }else{
                    if(events[i].events & EPOLLIN){
                        myevents[myevents_count].data.fd=events[i].data.fd;
                        myevents[myevents_count].events=events[i].events;
                        //epoll_ctl(server_socket,EPOLL_CTL_DEL,events[i].data.fd,NULL); 
                        handler[events[i].data.fd].setHandler(events[i].data.fd,tree_fd,pool);
                        
                        handler[events[i].data.fd].dealwithread();
                        myevents_count=(myevents_count+1)%1024;
                    }else if(events[i].events & EPOLLOUT){
                        handler[events[i].data.fd].setHandler(events[i].data.fd,tree_fd,pool);
                        
                        handler[events[i].data.fd].dealwithwrite();
                    }
                }
            } 
        }
    }