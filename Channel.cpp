#include "Channel.h"
/*
class Channel{
private:
    int fd_=-1;        //channel与fd一一对应
    Epoll *ep_=nullptr;     //channel对Epoll是多对一的关系，一个channnel只有一个Epoll
    bool inepoll_=false;        //channel是否已经添加到epoll树上，若已添加，调用epoll_ctl时用EPOLL)CTL_ADD,否则用EPOLL_CTL_MOD
    uint32_t events_=0;         //fd需要监视的事件，listenfd和clientfd需要监视EPOLLIN，clientfd需要监视EPOLLOUT
    uint32_t revents_=0;        //fd已发生的事件
public:
    Channel(Epoll *ep,int fd);  //构造函数
    ~Channel();                 //析构函数

    int fd();                   //返回fd成员
    void useet();               //采用边缘触发

    void enablereading();               //让epoll_wait监视fd的读事件
    void setinepoll();                  //把inepoll成员变量设置为true
    void setrevents(uint32_t ev);       //设置revents成员的值为参数ev
    bool inpoll();                      //返回inpoll成员的值
    uint32_t events();                  //返回events成员
    uint32_t revents();                 //返回revents成员
}
*/
Channel::Channel(Epoll *ep,int fd,bool islisten):ep_(ep),fd_(fd),islisten_(islisten) //构造函数
{
}
Channel::~Channel()                      //析构函数
{
    //不能关闭ep_,和fd_，因为这两个不属于channel，只能使用
}
int Channel::fd()                        //返回fd成员
{
    return fd_;
}
void Channel::useet()                    //采用边缘触发
{
    events_=events_|EPOLLET;
}
void Channel::enablereading()              //让epoll_wait监视fd的读事件
{
    events_=events_|EPOLLIN;
    ep_->updatechannel(this);

}
void Channel::setinepoll()                 //把inepoll成员变量设置为true
{
    inepoll_=true;
}
void Channel::setrevents(uint32_t ev)      //设置revents成员的值为参数ev
{
    revents_=ev;
}
bool Channel::inpoll()                     //返回inpoll成员的值
{
    return inepoll_;
}
uint32_t Channel::events()                 //返回events成员
{
    return events_;
}
uint32_t Channel::revents()                //返回revents成员
{
    return revents_;
}
void Channel::handleevent(Socket* servsock)
{
    if(revents_ & EPOLLRDHUP)                 //对方已关闭，有些系统监测不到，可以使用EPOLLIN,recv()返回0
    {
        printf("client(eventfd=%d) disconnected.\n",fd_);
        close(fd_);     //关闭客户端的fd
    }
    else if(revents_ & EPOLLIN|EPOLLPRI)      //接收缓冲区中有数据可读
    {
        if(islisten_==true)   //如果是listenfd有事件，表示有新的客户端连上来
        {
                    
        InetAddress clientaddr;   //创建存放客户端的地址和协议的对象

        // 注意，clientsock只能new出来，不能在栈上，否则析构函数会关闭fd。
        // 还有，这里new出来的对象没有释放，这个问题以后再解决。
        Socket *clientsock=new Socket(servsock->accept(clientaddr));
        printf("accept client(fd=%d,ip=%s,port=%d)ok\n",clientsock->fd(),clientaddr.ip(),clientaddr.port());

        //为新客户端连接准备读事件，并添加到epoll中
        Channel *clientchannel=new Channel(ep_,clientsock->fd(),false);
        clientchannel->useet();
        clientchannel->enablereading();
        //ep.addfd(clientsock->fd(),EPOLLIN|EPOLLET);         //客户端连上的fd用边缘出发              
        }
        else{
            char buffer[1024];
            while(true)
            {
                bzero(&buffer,sizeof(buffer));
                ssize_t nread = read(fd_,buffer,sizeof(buffer));
                if(nread>0) //成功读取到了数据
                {
                    //把接收到的报文内容原封不懂的发回去
                    printf("recv(eventfd=%d):%s\n",fd_,buffer);
                    send(fd_,buffer,strlen(buffer),0);
                }
                else if(nread==-1&& errno ==EINTR)  //读取数据的时候被信号中断，继续读取
                {
                    continue;
                }
                else if(nread==-1 && ((errno==EAGAIN)||(errno=EWOULDBLOCK)))    //全部的数据已读取完毕
                {
                    break;
                }
                else if(nread==0)   //客户端连接已断开
                {
                    printf("client(eventfd=%d) disconneted\n",fd_);
                    close(fd_); //关闭客户端的fd
                    break;
                }
            }
        }
    }
    else if(revents_ & EPOLLOUT)              //需要写
    {}
    else                                            //其他事件视为错误
    {
        printf("client(eventfd=%d)error\n",fd_);
        close(fd_);     //关闭客户端的fd
    }
}