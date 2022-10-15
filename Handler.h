#include <errno.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <regex>
#include <stdio.h>
#include <fstream>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include"threadpool.h"

class Handler{
    public:
    int tree_fd;
    int client_fd;
    int rest_have_tosend;
    int write_mod;
    int imglength;
    std::string File_type;
    char ImgBuffer[30000];
    char buf[256];
    char response_header[4096];
    char response_text[40960];
    threadpool<Handler> *pool;
    public:
        Handler();
        int getclientfd();
        ~Handler();
        void setHandler(int client_fd,int tree_fd,threadpool<Handler> *pool);
        void dealwithread();
        void dealwithwrite();
        void handle_read();
        void process();
        void dowrite(const char *buf,size_t data_size);
        void handle_write();
        //读取一行
        int get_line(int clnt_sock,char *buf,int size);
        void response_prepare(int clnt_sock,const char *path);
        void not_found(int clnt_sock);
};
