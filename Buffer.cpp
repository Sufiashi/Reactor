#include "Buffer.h"

Buffer::Buffer(uint16_t sep):sep_(sep)
{}
Buffer::~Buffer(){}
void Buffer::append(const char * data,size_t size)     //把数据·追加到buf_
{
    buf_.append(data,size);
}
size_t Buffer::size()                                  //返回buf的大小
{
    return buf_.size();
}
const char* Buffer::data()                            //返回buf的首地址
{
    return buf_.data();
}
void Buffer::clear()                                   //清空buf
{
    buf_.clear();
}

void Buffer::erase(size_t pos,size_t nn){
    buf_.erase(pos,nn);

}

void Buffer::appendwithsep(const char *data,size_t size){
    if(sep_==0){
        buf_.append(data,size);
    }
    else if(sep_==1){
        buf_.append((char*)&size,4);       //处理报文头部
        buf_.append(data,size);            //处理报文内容
    }
    else{
        buf_.append("\r\n\r\n");       //处理报文头部
        buf_.append(data,size);            //处理报文内容
    }
}

bool Buffer::pickmessage(std::string &ss){
    if (buf_.size()==0) return false;

    if (sep_==0)                  // 没有分隔符。
    {
        ss=buf_;
        buf_.clear();
    }
    else if (sep_==1)          // 四字节的报头。
    {
        int len;
        memcpy(&len,buf_.data(),4);             // 从buf_中获取报文头部。

        if (buf_.size()<len+4) return false;     // 如果buf_中的数据量小于报文头部，说明buf_中的报文内容不完整。

        ss=buf_.substr(4,len);                        // 从buf_中获取一个报文。
        buf_.erase(0,len+4);                          // 从buf_中删除刚才已获取的报文。
    }

    return true; 
}


