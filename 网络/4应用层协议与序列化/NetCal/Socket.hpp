#pragma once
#include "Common.hpp"
#include "InetAddr.hpp"
#include "log.hpp"

const int defaultbacklog = 5;

using namespace LogModule;
namespace SocketModule
{
    class Socket
    {
    public:
        virtual ~Socket() = 0;
        virtual void SocketOrDie() = 0;
        virtual void BindOrDie(uint16_t port) = 0;
        virtual void ListenOrDie(int backlog) = 0;
        virtual std::shared_ptr<Socket> AcceptOrDie(InetAddr *addr) = 0;
        virtual void Close() = 0;
        virtual void read() = 0;
        virtual void write() = 0;

    public:
        void BuildTCPSocketMethod(uint16_t port, int backlog = defaultbacklog)
        {
            SocketOrDie();
            BindOrDie(port);
            ListenOrDie(backlog);
        }

    private:
    };

    class TCPSocket : public Socket
    {
    public:
        TCPSocket()
        {
        }
        void SocketOrDie() override
        {
            _sockfd = socket(AF_INET, SOCK_STREAM, 0);
            if (_sockfd < 0)
            {
                LOG(Loglevel::FATAL) << "socket err";
                exit(SOCKET_ERR);
            }
        }

        void BindOrDie(uint16_t port) override
        {
            InetAddr addr(port);
            int n = bind(_sockfd, addr.NetAddrPtr(), addr.NetAddrlen());
            if (n < 0)
            {
                LOG(Loglevel::ERROR) << "bind err";
                exit(BIND_ERR);
            }
        }

        void ListenOrDie(int backlog = defaultbacklog) override
        {
            int n = listen(_sockfd, backlog);
            if (n < 0)
            {
                LOG(Loglevel::ERROR) << "listen err";
                exit(LISTEN_ERR);
            }
        }

        std::shared_ptr<Socket> AcceptOrDie(InetAddr *addr) override
        {
            struct sockaddr_in peer;
            socklen_t len = sizeof(peer);
            int n = accept(_sockfd, CONV(peer), &len);
            if (n < 0)
            {
                LOG(Loglevel::WARNING) << "accept err";
                return nullptr;
            }
            addr->SetAddr(peer);
            return std::make_shared<TCPSocket>(n);
        }

        void Close() override
        {
            int n = close(_sockfd);
            if (n < 0)
            {
                LOG(Loglevel::ERROR) << "close err";
                return;
            }
        }

    private:
        int _sockfd;
    };
}
