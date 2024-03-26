#include "Epoll.h" 

Epoll::Epoll(){
    if((epollfd_ = epoll_create(1))==-1)  //创建epoll句柄（红黑树）
    {
        printf("epoll_create failed(%d)\n",errno);exit(-1);
    }


}
Epoll::~Epoll(){
    close(epollfd_);
}
void Epoll::addfd(int fd,uint32_t op){
    //为服务端的listenfd准备读事件
    struct epoll_event ev;         //声明事件的数据结构
    ev.data.fd=fd;    //指定事件自定义数据，会随着epoll_wait()返回的事件一并返回
    ev.events=op;      //让epoll监视listenfd的读事件，采用水平触发

    if(epoll_ctl(epollfd_,EPOLL_CTL_ADD,fd,&ev)==-1)  //把需要监视的listenfd和他的事件加入epollfd中
    {
        printf("epoll_ctl failed(%d)",errno);exit(-1);
    }
}
void Epoll::updatechannel(Channel *ch){
    //为服务端的listenfd准备读事件
    struct epoll_event ev;         //声明事件的数据结构
    ev.data.ptr=ch;    //指定事件自定义数据，会随着epoll_wait()返回的事件一并返回
    ev.events=ch->events();      //让epoll监视listenfd的读事件，采用水平触发

    if(ch->inpoll())
    {
        if((epoll_ctl(epollfd_,EPOLL_CTL_MOD,ch->fd(),&ev))==-1)
        {
            perror("epoll_ctl() failed\n"); exit(-1);
        }
    }
    else        //如果channel不在树上
    {
        if(epoll_ctl(epollfd_,EPOLL_CTL_ADD,ch->fd(),&ev)==-1)  //把需要监视的listenfd和他的事件加入epollfd中
        {
        printf("epoll_ctl failed(%d)",errno);exit(-1);
        }
        ch->setinepoll();       //把channel的inepoll设置为true
    }
}
std::vector<Channel*> Epoll::loop(int timeout){
    std::vector<Channel*> channels;
    bzero(events_,sizeof(events_));       //初始化数组
    int infds=epoll_wait(epollfd_,events_,MaxEvents,timeout);    //等待监视的fd有事件发生

     //返回失败
    if(infds < 0)
    {   // EBADF ：epfd不是一个有效的描述符。
        // EFAULT ：参数events指向的内存区域不可写。
        // EINVAL ：epfd不是一个epoll文件描述符，或者参数maxevents小于等于0。
        // EINTR ：阻塞过程中被信号中断，epoll_pwait()可以避免，或者错误处理中，解析error后重新调用epoll_wait()。
        // 在Reactor模型中，不建议使用信号，因为信号处理起来很麻烦，没有必要。------ 陈硕
        perror("epoll_wait() failed");exit(-1);
    }

        //超时
    if(infds == 0)
    {
        //如果epoll_wait超时，表示系统空闲，返回的channels将为空
        return channels;
    }
    //如果infds>0，表示有时间发生
    for(int ii=0;ii<infds;ii++){
        Channel* ch=(Channel*)events_[ii].data.ptr;//取出已发生事件的channel
        ch->setrevents(events_[ii].events);                 //设置channel的revents乘员
        channels.push_back(ch);
    }
    return channels;
}
//从红黑树上删除channel
void Epoll::removechannel(Channel* ch){
    if(ch->inpoll())        //如果channel已经在树上了
    {
        printf("removechannel()\n");
        if(epoll_ctl(epollfd_,EPOLL_CTL_DEL,ch->fd(),0)==-1)  //把需要监视的listenfd和他的事件加入epollfd中
        {
            printf("epoll_ctl failed(%d)",errno);exit(-1);
        }
    }
}