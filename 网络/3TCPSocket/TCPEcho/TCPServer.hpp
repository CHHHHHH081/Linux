#pragma once
#include "InetAddr.hpp"
#include "log.hpp"
#include "Common.hpp"

using namespace LogModule;
using func_t = std::function<void()>;

class TCPServer : public NoCopy
{
public:
    TCPServer(uint16_t port, func_t func)
        : _port(port), _func(func), _isrunning(false)
    {
    }

    void Init()
    {
        _listensockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (_listensockfd < 0)
        {
            LOG(Loglevel::FATAL) << "socket err";
            exit(SOCKET_ERR);
        }
        InetAddr addr(_port);
        int n = bind(n, CONV(addr), addr.NetAddrlen());
        if (n < 0)
        {
            LOG(Loglevel::ERROR) << "bind err";
            exit(BIND_ERR);
        }
        n = listen(_listensockfd, 128);
        if (n < 0)
        {
            LOG(Loglevel::ERROR) << "listen err";
            exit(LISTEN_ERR);
        }
    }

    void Start()
    {
        _isrunning = true;
        while (1)
        {
            struct sockaddr_in peer;
            socklen_t len = sizeof(peer);
            _sockfd = accept(_listensockfd, CONV(peer), &len);
            if (_sockfd < 0)
            {
                LOG(Loglevel::WARNING) << "获取连接失败, 尝试再次获取";
                continue;
            }
            char buf[1024];
            int n = read(_sockfd, buf, sizeof(buf) - 1);
            if (n < 0)
            {
                LOG(Loglevel::ERROR) << "读取失败";
            }
            else if (n == 0)
            {
            }
            buf[n] = 0;
            LOG(Loglevel::INFO) << buf;
            write(_sockfd, buf, n);
        }
    }

private:
    uint16_t _port;
    func_t _func;
    bool _isrunning;
    int _listensockfd; // 监听套接字描述符
    int _sockfd;       // 用于读写
};
