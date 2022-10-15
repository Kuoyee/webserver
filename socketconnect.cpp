#include"socketconnect.h"

socketconnect::socketconnect(int port){
    //创建套接字
    listen_fd=socket(AF_INET,SOCK_STREAM | SOCK_NONBLOCK,IPPROTO_TCP);
    this->port=port;
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));  //每个字节都用0填充
    serv_addr.sin_family = AF_INET;  //使用IPv4地址
    serv_addr.sin_addr.s_addr =INADDR_ANY;  //具体的IP地址
    serv_addr.sin_port = htons(port);  //端口
    //bind将套接字、IP、端口绑定
    bind(listen_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    //监听
    listen(listen_fd,120);
    std::cout<<"等待连接 "<<listen_fd<<std::endl;
}
int socketconnect::getlisten_fd(){
    return listen_fd;
}

socketconnect::~socketconnect(){
    close(listen_fd);
}