#include <sys/socket.h>
#include <arpa/inet.h>//////
#include<iostream>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
class socketconnect{
    private:
       int listen_fd;
       int port;
    public:
       socketconnect(int port);
       int getlisten_fd();
       ~socketconnect();
};