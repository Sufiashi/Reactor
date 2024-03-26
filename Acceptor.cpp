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

Acceptor::Acceptor(EventLoop* loop,const std::string& ip,uint16_t port)
:loop_(loop),servsock_(createnonblocking()),acceptchannel_(loop_,servsock_.fd())
{
    //创建服务端用于监听的listenfd
    //servsock_=new Socket(createnonblocking());      //必须是new出的堆对象，如果是直接创建的栈对象则在构造函数结束之后就会被关闭
    InetAddress servaddr(ip,port);  //服务端的地址和协议
    servsock_.setreuseaddr(true);
    servsock_.setreuseport(true);
    servsock_.settcpnodelay(true);
    servsock_.setkeepalive(true);
    servsock_.bind(servaddr);
    servsock_.listen();
    
    acceptchannel_.enablereading();
    acceptchannel_.setreadcallback(std::bind(&Acceptor::newconnection,this));
}

Acceptor::~Acceptor(){
    //delete servsock_;
    //delete acceptchannel_;
}

// 处理新客户端连接请求。
void Acceptor::newconnection()    
{
    InetAddress clientaddr;             // 客户端的地址和协议。
    
    std::unique_ptr<Socket> clientsock(new Socket(servsock_.accept(clientaddr)));

    clientsock->setipport(clientaddr.ip(),clientaddr.port());

   

    newconnectioncb_(std::move(clientsock));        //回调TcpServer::newconnection()
}

void Acceptor::setnewconnectioncb(std::function<void(std::unique_ptr<Socket>)> fn){
    newconnectioncb_=fn;
}