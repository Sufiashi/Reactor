/*
*程序名：tcpepoll.cpp,此程序用于演示采用epoll模型实现网络通信的服务端
*作者：Sufia
*/
#include "TcpServer.h"

int main(int argc,char* argv[]){
    if(argc!=3){
        printf("usage:./tcpepoll ip port\n");
        printf("example:./tcpepoll 192.168.93.5 5005\n\n");
        return -1;
    }

    TcpServer tcpserver(argv[1],atoi(argv[2]));
   
    
    tcpserver.start();
   

    return 0;
}
