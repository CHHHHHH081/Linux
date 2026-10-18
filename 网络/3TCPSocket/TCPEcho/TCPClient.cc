#include "log.hpp"
#include "Common.hpp"
#include "InetAddr.hpp"
#include <string>

using namespace LogModule;

void Usage()
{
    LOG(Loglevel::WARNING) << "usage err";
    exit(USAGE_ERR);
}

int main(int argc, const char *argv[])
{
    if (argc != 3)
    {
        Usage();
    }
    std::string ip = argv[1];
    uint16_t port = std::stoi(argv[2]);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        LOG(Loglevel::ERROR) << "socket fail";
        exit(SOCKET_ERR);
    }

    InetAddr addr(ip, port);
    int n = connect(sockfd, addr.NetAddrPtr(), addr.NetAddrlen());
    if (n < 0)
    {
        LOG(Loglevel::ERROR) << "connect err";
        exit(CONNECT_ERR);
    }
    while (1)
    {
        std::string line;
        std::cout << "Please Enter# ";
        std::getline(std::cin, line);
        write(sockfd, line.c_str(), line.size());

        char buf[1024];
        int n = read(sockfd, buf, sizeof(buf) - 1);
        if (n < 0)
        {
            LOG(Loglevel::ERROR) << "read fail";
            exit(READ_ERR);
        }
        else if (n == 0)
        {
            LOG(Loglevel::INFO) << addr.AddrString() << "已退出...";
            close(sockfd);
        }
        else
        {
            buf[n] = 0;
            std::cout << "Server Echo: " << buf << std::endl;
        }
    }
    return 0;
}