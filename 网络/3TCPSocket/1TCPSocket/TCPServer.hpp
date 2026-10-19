#pragma once
#include "InetAddr.hpp"
#include "log.hpp"
#include "Common.hpp"
#include "threadpool.hpp"

using namespace LogModule;
using func_t = std::function<std::string(std::string, InetAddr)>;

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
        int n = bind(_listensockfd, CONV(addr), addr.NetAddrlen());
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
        LOG(Loglevel::INFO) << "listen success";
    }

    class ThreadData
    {
    public:
        ThreadData(int sockfd, InetAddr addr, TCPServer *tsvr)
            : _sockfd(sockfd), _addr(addr), _tsvr(tsvr)
        {
        }
        int _sockfd;
        InetAddr _addr;
        TCPServer *_tsvr;
    };

    void Service(int sockfd, InetAddr peer)
    {
        while (1)
        {
            char buf[1024];
            int n = read(sockfd, buf, sizeof(buf) - 1);
            if (n < 0)
            {
                LOG(Loglevel::ERROR) << "读取失败";
                return;
            }
            else if (n == 0)
            {
                LOG(Loglevel::INFO) << peer.AddrString() << "已退出...";
                close(sockfd);
                return;
            }
            buf[n] = 0;
            std::string ret = _func(std::string(buf), peer);
            LOG(Loglevel::INFO) << peer.AddrString() << "# " << ret;
            write(sockfd, ret.c_str(), ret.size());
        }
    }

    static void *Routine(void *args)
    {
        pthread_detach(pthread_self());
        ThreadData *td = static_cast<ThreadData *>(args);
        td->_tsvr->Service(td->_sockfd, td->_addr);
        delete td;
        return nullptr;
    }

    void Start()
    {
        _isrunning = true;
        while (_isrunning)
        {
            struct sockaddr_in peer;
            socklen_t len = sizeof(peer);
            // 获取连接, 没有就阻塞
            int sockfd = accept(_listensockfd, CONV(peer), &len);
            if (sockfd < 0)
            {
                LOG(Loglevel::WARNING) << "获取连接失败, 尝试再次获取";
                continue;
            }
            InetAddr addr(peer);
            LOG(Loglevel::INFO) << "获取连接成功";

            // 进程版
            //  int n = fork();
            //  if (n < 0)
            //  {
            //      LOG(Loglevel::ERROR) << "fork err";
            //      exit(FORK_ERR);
            //  }
            //  else if (n == 0)
            //  {
            //      // 子进程
            //      close(_listensockfd); // 子进程不再使用listen套接字
            //      if (fork() > 0)
            //          exit(OK);
            //      // 孙子进程
            //      Service(sockfd, addr);
            //      exit(OK);
            //  }
            //  else
            //  {
            //      // 父进程
            //      close(sockfd);
            //      pid_t pid = waitpid(n, nullptr, 0);
            //      (void)pid;
            //  }

            // 多线程版
            ThreadData *td = new ThreadData(sockfd, addr, this);
            pthread_t tid;
            pthread_create(&tid, nullptr, Routine, td);
        }
        _isrunning = false;
    }

private:
    uint16_t _port;
    func_t _func;
    bool _isrunning;
    int _listensockfd; // 监听套接字描述符
};
