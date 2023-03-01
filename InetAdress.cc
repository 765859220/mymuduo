#include "InetAddress.h"

#include <strings.h>
#include <string.h>

InetAddress::InetAddress(uint16_t port, std::string ip)
{
    bzero(&addr_, sizeof addr_);//addr_清0了，相当于memset
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);//转成网络字节序
    addr_.sin_addr.s_addr = inet_addr(ip.c_str());
}

std::string InetAddress::toIp() const//打印IP地址
{
    //addr_
    char buf[64] = {0};
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);//读出整数的表示网络字节序转成本地字节序
    return buf;
}

std::string InetAddress::toIpPort() const
{
    //ip:port
    char buf[64] = {0};
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);
    size_t end = strlen(buf);
    uint16_t port = ntohs(addr_.sin_port);
    sprintf(buf+end, ":%u", port);
    return buf;
}

uint16_t InetAddress::toPort() const
{
    return ntohs(addr_.sin_port);
}

// #include <iostream>
// int main()
// {
//     InetAddress addr(8080);
//     std::cout << addr.toIpPort() << std::endl;

//     return 0;
// }
