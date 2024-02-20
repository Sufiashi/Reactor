#include "TcpServer.h"

TcpServer::TcpServer(const std::string &ip,const uint16_t port){
    //创建服务端用于监听的listenfd
    acceptor_=new Acceptor(&loop_,ip,port);
}
TcpServer::~TcpServer(){
    delete acceptor_;
}

void TcpServer::start(){
    loop_.run();
}