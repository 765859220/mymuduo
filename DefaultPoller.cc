#include "Poller.h"
#include "EPollPoller.h"

#include <stdlib.h>

//包含了具体实现类的头文件
Poller* Poller::newDefaultPoller(EventLoop *loop)
{
    // 如果用户在环境变量中设置了要用poll
    if (::getenv("MUDUO_USE_POLL"))
    {
        return nullptr;//生成poll的实例
    }
    else
    {
        return new EPollPoller(loop);//生成epoll的实例
    }
}
