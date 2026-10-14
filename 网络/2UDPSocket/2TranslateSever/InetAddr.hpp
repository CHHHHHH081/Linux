#pragma once
#include <iostream>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

class InetAddr
{
public:
    InetAddr(struct sockaddr_in addr)
        : _addr(addr)
    {
        _ip = inet_ntoa(addr.sin_addr);
        _port = ntohs(addr.sin_port);
    }

    std::string IP() { return _ip; }
    uint16_t Port() { return _port; }
    std::string AddrString() { return _ip + ":" + std::to_string(_port); }

private:
    struct sockaddr_in _addr;
    std::string _ip;
    uint16_t _port;
};
