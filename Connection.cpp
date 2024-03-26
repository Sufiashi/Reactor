
#include "Connection.h"


Connection::Connection(EventLoop*loop,std::unique_ptr<Socket> clientsock)
            :loop_(loop),clientsock_(std::move(clientsock)),disconnect_(false),clientchannel_(new Channel(loop_,clientsock_->fd())) 
{
    // 为新客户端连接准备读事件，并添加到epoll中。   
    clientchannel_->setreadcallback(std::bind(&Connection::onmessage,this));
    clientchannel_->setclosecallback(std::bind(&Connection::closecallback,this));
    clientchannel_->seterrorcallback(std::bind(&Connection::errorcallback,this));
    clientchannel_->setwritecallback(std::bind(&Connection::writecallback,this));
    clientchannel_->useet();                 // 客户端连上来的fd采用边缘触发。
    clientchannel_->enablereading();   // 让epoll_wait()监视clientchannel的读事件
}

Connection::~Connection()
{
}

int Connection::fd() const                              // 返回客户端的fd。
{
    return clientsock_->fd();
}

std::string Connection::ip() const                   // 返回客户端的ip。
{
    return clientsock_->ip();
}

uint16_t Connection::port() const                  // 返回客户端的port。
{
    return clientsock_->port();
}

void Connection::closecallback()                    // TCP连接关闭（断开）的回调函数，供Channel回调。
{
    disconnect_=true;
    closecallback_(shared_from_this());
}

void Connection::errorcallback()                    // TCP连接错误的回调函数，供Channel回调。
{
    disconnect_=true;
    errorcallback_(shared_from_this());
}

void Connection::setclosecallback(std::function<void(spConnection)>fn){
    closecallback_=fn;
}
void Connection::seterrorcallback(std::function<void(spConnection)>fn){
    errorcallback_=fn;
}

// 处理对端发送过来的消息。
void Connection::onmessage()
{
    char buffer[1024];
    while (true)             // 由于使用非阻塞IO，一次读取buffer大小数据，直到全部的数据读取完毕。
    {    
        bzero(&buffer, sizeof(buffer));
        ssize_t nread = read(fd(), buffer, sizeof(buffer));
        if (nread > 0)      // 成功的读取到了数据。
        {
            // 把接收到的报文内容原封不动的发回去。
            //printf("recv(eventfd=%d):%s\n",fd(),buffer);
            //send(fd(),buffer,strlen(buffer),0);
            inputbuffer_.append(buffer,nread);      //读取到的数据先存放在接收缓冲区中

        } 
        else if (nread == -1 && errno == EINTR) // 读取数据的时候被信号中断，继续读取。
        {  
            continue;
        } 
        else if (nread == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) // 全部的数据已读取完毕。
        {

            std::string message; 
            while(true)
            {
                //运算
                ////////////////////////////
                //可以把下列代码封装在Buffer类中，还可以支持固定长度，指定报文长度和分隔符等多种格式
                if(inputbuffer_.pickmessage(message)==false)break;
                /////////////////////////////////////
                //printf("message (eventfd=%d):%s\n",fd(),message.c_str());
                lastatime_=Timestamp::now();                //更新Connection的时间戳
                //std::cout<<"lastatime_="<<lastatime_.tostring()<<std::endl; 
                onmessagecallback_(shared_from_this(),message);       //回调TcpServer::onmessage()
                //将接收到的发送到发送缓冲区
            }      
            break;
            
        } 
        else if (nread == 0)  // 客户端连接已断开。
        {  
            clientchannel_->remove();    //从事件循环中删除channel
            closecallback();            //回调TcpServer::closecallback()
            break;
        }
    }
}
void Connection::setonmessagecallback(std::function<void(spConnection,std::string&)>fn){
    onmessagecallback_=fn;
}

void Connection::send(const char* data,size_t size){
    if(disconnect_==true){printf("客户端连接已经断开，send直接返回\n");return;}

    if(loop_->isinloopthread())  //判断当前进程是否是IO线程（事件循环线程）
    {
        //如果是IO线程，直接执行发送数据的操作       
        sendinloop(data,size);
    }
    else{
        //如果不是IO线程，把发送数据的操作交给IO线程操作
        loop_->queueinloop(std::bind(&Connection::sendinloop,this,data,size));
    }
    
}
void Connection::sendinloop(const char* data,size_t size) 
{
    outputbuffer_.appendwithsep(data,size);
    clientchannel_->enablewriting(); //注册写事件
}


void Connection::writecallback(){
    int writen=::send(fd(),outputbuffer_.data(),outputbuffer_.size(),0);      //把发送缓冲区中的数据全部发送出去
    if(writen>0)outputbuffer_.erase(0,writen);              //发送缓冲区中删除已发送的字节

    if(outputbuffer_.size()==0){
        clientchannel_->disablewriting();//没有数据，不在关注写事件
        sendcompletecallback_(shared_from_this());
    }
}

void Connection::setsendcompletecallback(std::function<void(spConnection)>fn){
    sendcompletecallback_=fn;
}
bool Connection::timeout(time_t now,int val){
    return now-lastatime_.toint()>val;
}
  
