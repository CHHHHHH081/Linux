#pragma once
#include <iostream>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "log.hpp"
#include <functional>
#include "Mutex.hpp"
#include "InetAddr.hpp"

using namespace std;
using namespace LogModule;

// 传入的func内部会调用路由的函数
using func_t = function<void(int, const string &, InetAddr &addr)>;

class UDPSocket
{
public:
    UDPSocket(uint16_t port, func_t handler)
        : _port(port), _isrunning(false), _handler(handler)
    {
        // 需要传入port和处理函数handler
    }

    void Init()
    {
        _sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (_sockfd < 0)
        {
            LOG(Loglevel::ERROR) << "server socket fail!";
            exit(1);
        }
        LOG(Loglevel::INFO) << "server socket success!";

        struct sockaddr_in addr;
        bzero(&addr, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(_port);
        addr.sin_addr.s_addr = INADDR_ANY;

        int n = bind(_sockfd, (const sockaddr *)&addr, sizeof(addr));
        if (n < 0)
        {
            LOG(Loglevel::ERROR) << "server bind fail!";
            exit(2);
        }
        LOG(Loglevel::INFO) << "server bind success!";
    }

    void Start()
    {
        _isrunning = true;
        while (_isrunning)
        {
            char buf[1024];
            // 循环接收+发送
            // peer输出型参数, 得到的是client端的信息
            struct sockaddr_in peer;
            socklen_t len = sizeof(peer);
            ssize_t n = recvfrom(_sockfd, buf, sizeof(buf) - 1, 0, (struct sockaddr *)&peer, &len);
            if (n < 0)
            {
                LOG(Loglevel::ERROR) << "server recv error!";
                exit(3);
            }
            // 拿到client端地址
            InetAddr addr(peer);
            string addrstring = addr.AddrString();
            // 末位补\0
            buf[n] = 0;
            string s = buf;
            // 将收到的信息buf直接交给路由处理, _handler内部有路由的函数, server端只负责收信息, 而路由则负责把信息发给所有用户
            _handler(_sockfd, s, addr);
        }
    }
    ~UDPSocket()
    {
    }

private:
    int _sockfd;
    uint16_t _port;
    bool _isrunning;
    func_t _handler;
};
