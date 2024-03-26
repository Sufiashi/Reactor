/*
*程序名：echoserver.cpp,此程序用于演示采用epoll模型实现网络通信的服务端
*作者：Sufia
*/
#include "EchoServer.h"
#include <signal.h>

// 1、设置2和15的信号。
// 2、在信号处理函数中停止主从事件循环和工作线程。
// 3、服务程序主动退出。 


EchoServer *echoserver;
void Stop(int sig){
    printf("sig=%d\n",sig);
    //调用EchoServer::Stop()停止服务
    echoserver->Stop();
    printf("echoserver已停止\n");
    delete echoserver;          //
    exit(0);
}
int main(int argc,char* argv[]){
    if(argc!=3){
        printf("usage:./echoserver ip port\n");
        printf("example:./echoserver 192.168.93.5 5005\n\n");
        return -1;
    }
    signal(SIGTERM,Stop);
    signal(SIGINT,Stop);
    //TcpServer tcpserver(argv[1],atoi(argv[2]));
    
    //tcpserver.start();
    echoserver=new EchoServer(argv[1],atoi(argv[2]),30,0);
    echoserver->Start();

    return 0;
}
