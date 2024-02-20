#include "Acceptor.h"

/*
class Acceptor
{
private:
    EventLoop *loop_;           //Acceptor对应一个事件循环，在构造函数中传入，只能使用，但不属于这个类
    Socket *servsock_;          //服务端用于监听的socket，在构造函数用创建
    Channel * acceptchannel_;   //Acceptor对应的channel，在构造函数中创建
public:
    Acceptor(EventLoop *loop,const std::string& ip,uint16_t port);
    ~Acceptor();
};
*/

Acceptor::Acceptor(EventLoop *loop,const std::string& ip,uint16_t port):loop_(loop)
{
    //创建服务端用于监听的listenfd
    servsock_=new Socket(createnonblocking());      //必须是new出的堆对象，如果是直接创建的栈对象则在构造函数结束之后就会被关闭
    InetAddress servaddr(ip,port);  //服务端的地址和协议
    servsock_->setreuseaddr(true);
    servsock_->setreuseport(true);
    servsock_->settcpnodelay(true);
    servsock_->setkeepalive(true);
    servsock_->bind(servaddr);
    servsock_->listen();
    
    acceptchannel_=new Channel(loop_,servsock_->fd());
    acceptchannel_->enablereading();
    acceptchannel_->setreadcallback(std::bind(&Channel::newconnection,acceptchannel_,servsock_));
}

Acceptor::~Acceptor(){
    delete servsock_;
    delete acceptchannel_;
}