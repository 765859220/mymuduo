#pragma once

#include "noncopyable.h"

class InetAddress;

//封装socket fd
class Socket : noncopyable
{
public:
    explicit Socket(int sockfd)
        : sockfd_(sockfd)
    {}

    ~Socket();

    int fd() const { return sockfd_; }
    // TCP编程的三步：bind、listen、accept
    void bindAddress(const InetAddress &localaddr);
    void listen();
    // TCP编程的三步：bind、listen、accept
    int accept(InetAddress *peeraddr);

    void shutdownWrite();

    void setTcpNoDelay(bool on);//直接发送，数据不进行TCP缓存 
    void setReuseAddr(bool on);
    void setReusePort(bool on);
    void setKeepAlive(bool on);
private:
    const int sockfd_;
};
