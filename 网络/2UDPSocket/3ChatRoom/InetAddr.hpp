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
        char ipbuffer[64];
        inet_ntop(AF_INET, &_addr.sin_addr, ipbuffer, sizeof(ipbuffer));
        _ip = ipbuffer;
        _port = ntohs(addr.sin_port);
        _addrstring = _ip + ":" + std::to_string(_port);
    }
    InetAddr(const std::string &ip, uint16_t port)
        : _ip(ip), _port(port)
    {
        memset(&_addr, 0, sizeof(_addr));
        _addr.sin_family = AF_INET;
        inet_pton(AF_INET, ip.c_str(), &_addr.sin_addr);
        _addr.sin_port = htons(port);
    }

    std::string IP() const { return _ip; }
    uint16_t Port() const { return _port; }
    const struct sockaddr_in &GetAddr() const { return _addr; }
    std::string AddrString() const { return _addrstring; }
    bool operator==(const InetAddr &addr) const { return _addrstring == addr._addrstring; }

private:
    struct sockaddr_in _addr;
    std::string _ip;
    uint16_t _port;
    std::string _addrstring;
};
