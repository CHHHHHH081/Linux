#include <iostream>
#include <string>
#include <cstring>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "log.hpp"

using namespace LogModule;

int main(int argc, char *argv[])
{
    Enable_Console_Stretage();

    // 需要输入server端的ip + port
    if (argc != 3)
    {
        LOG(Loglevel::ERROR) << "Client Usage: " << argv[0] << " server_ip server_port";
        exit(1);
    }

    // 读取argv, argv[1]:server_ip, argv[2]:server_port
    std::string server_ip = argv[1];
    uint16_t server_port = std::stoi(argv[2]);

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        LOG(Loglevel::ERROR) << "client socket fail!";
        exit(2);
    }
    LOG(Loglevel::NORMAL) << "client socket success!";

    // 用刚刚读取的server端的socket填充sockaddr
    struct sockaddr_in server;
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(server_port);
    server.sin_addr.s_addr = inet_addr(server_ip.c_str());

    while (1)
    {
        // 循环发送+接收
        std::string input;
        std::cout << "Please Enter# ";
        std::getline(std::cin, input);
        ssize_t n = sendto(sockfd, input.c_str(), input.size(), 0, (struct sockaddr *)&server, sizeof(server));
        if (n < 0)
        {
            LOG(Loglevel::ERROR) << "client send error!";
            exit(1);
        }
        LOG(Loglevel::NORMAL) << "client send success!";

        // 发送完成，准备接收回显
        char buf[1024];
        struct sockaddr_in peer;
        socklen_t len = sizeof(peer);
        ssize_t m = recvfrom(sockfd, buf, sizeof(buf) - 1, 0, (struct sockaddr *)&peer, &len);
        if (m > 0)
        {
            buf[m] = 0;
            std::cout << buf << std::endl;
        }
    }
    return 0;
}
