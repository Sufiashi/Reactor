#include "Channel.h"

Channel::Channel(EventLoop* loop,int fd):loop_(loop),fd_(fd)      // 构造函数。
{

}

Channel::~Channel()                           // 析构函数。 
{
    // 在析构函数中，不要销毁loop_，也不能关闭fd_，因为这两个东西不属于Channel类，Channel类只是需要它们，使用它们而已。
}

int Channel::fd()                                            // 返回fd_成员。
{
    return fd_;
}

void Channel::useet()                                    // 采用边缘触发。
{
    events_=events_|EPOLLET;
}

void Channel::enablereading()                     // 让epoll_wait()监视fd_的读事件。
{
    events_|=EPOLLIN;
    loop_->updatechannel(this);
}
void Channel::disablereading()                     // 取消读事件
{
    events_&=~EPOLLIN;
    loop_->updatechannel(this);
}
void Channel::enablewriting()                     // 注册写事件
{
    events_|=EPOLLOUT;
    loop_->updatechannel(this);
}
void Channel::disablewriting()                     // 取消写事件。
{
    events_&=~EPOLLOUT;
    loop_->updatechannel(this);
}
void Channel::setinepoll()                           // 把inepoll_成员的值设置为true。
{
    inepoll_=true;
}

void Channel::setrevents(uint32_t ev)         // 设置revents_成员的值为参数ev。
{
    revents_=ev;
}

bool Channel::inpoll()                                  // 返回inepoll_成员。
{
    return inepoll_;
}

uint32_t Channel::events()                           // 返回events_成员。
{
    return events_;
}

uint32_t Channel::revents()                          // 返回revents_成员。
{
    return revents_;
} 

// 事件处理函数，epoll_wait()返回的时候，执行它。
void Channel::handleevent()
{
    if (revents_ & EPOLLRDHUP)                     // 对方已关闭，有些系统检测不到，可以使用EPOLLIN，recv()返回0。
    {
        remove();           //从事件中删除Channel
        closecallback_();   // 设置fd_读事件的回调函数。
    }                                //  普通数据  带外数据    
    else if (revents_ & (EPOLLIN|EPOLLPRI))   // 接收缓冲区中有数据可以读。
    {
        //printf("EPOLLIN|EPOLLPRI\n");
        readcallback_();
    }
    else if (revents_ & EPOLLOUT)                  // 有数据需要写，暂时没有代码，以后再说。
    {
        //printf("EPOLLOUT\n");
        writecallback_();                   //回调Connection::writecallback()
    }
    else                                                                   // 其它事件，都视为错误。
    {
        remove();               //从事件循环中删除Channel
        errorcallback_();       //回调Connection::errorcallback()
    }
}





 // 设置fd_读事件的回调函数。
 void Channel::setreadcallback(std::function<void()> fn)    
 {
    readcallback_=fn;
 }

  // 设置f关闭fd的回调函数。
 void Channel::setclosecallback(std::function<void()> fn)    
 {
    closecallback_=fn;
 }
  // 设置fd发生错误的回调函数。
 void Channel::seterrorcallback(std::function<void()> fn)    
 {
    errorcallback_=fn;
 }

 void Channel::setwritecallback(std::function<void()> fn){
    writecallback_=fn;
 }
// 取消全部的事件
void Channel::disableall()
{
    events_=0;
    loop_->updatechannel(this);
}
// 从事件循环中删除Channel                        
void Channel::remove()
{
    disableall();                   //取消全部的事件
    loop_->removechannel(this);     //从红黑树上删除fd
}                            