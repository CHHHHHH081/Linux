#include <iostream>
#include <string>
#include <cstring>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <functional>
#include "log.hpp"
#include "Thread.hpp"

using namespace LogModule;
using namespace ThreadModule;
using task_t = std::function<void()>;

int sockfd = 0;
std::string server_ip;
uint16_t server_port = 0;
pthread_t id;

void Receive()
{
    while (1)
    {
        char buf[1024];
        struct sockaddr_in peer;
        socklen_t len = sizeof(peer);
        ssize_t m = recvfrom(sockfd, buf, sizeof(buf) - 1, 0, (struct sockaddr *)&peer, &len);
        if (m > 0)
        {
            buf[m] = 0;
            std::cerr << buf << std::endl;
        }
    }
}
void Send()
{
    struct sockaddr_in server;
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(server_port);
    server.sin_addr.s_addr = inet_addr(server_ip.c_str());
    const std::string sign_in = "sign_in";
    sendto(sockfd, sign_in.c_str(), sign_in.size(), 0, (struct sockaddr *)&server, sizeof(server));
    while (1)
    {
        std::string input;
        std::cout << "Please Enter# ";
        std::getline(std::cin, input);
        ssize_t n = sendto(sockfd, input.c_str(), input.size(), 0, (struct sockaddr *)&server, sizeof(server));
        if (n < 0)
        {
            LOG(Loglevel::ERROR) << "client send error!";
            exit(1);
        }
        if (input == "QUIT")
        {
            exit(0);
        }
    }
}

int main(int argc, char *argv[])
{
    Enable_Console_Stretage();

    // 需要输入server端的ip + port
    if (argc != 3)
    {
        LOG(Loglevel::ERROR) << "Client Usage: " << argv[0] << " server_ip server_port";
        exit(1);
    }

    // 1.读取argv, argv[1]:server_ip, argv[2]:server_port
    server_ip = argv[1];
    server_port = std::stoi(argv[2]);

    // 2.创建socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        LOG(Loglevel::ERROR) << "client socket fail!";
        exit(2);
    }
    LOG(Loglevel::INFO) << "client socket success!";

    // client端每个用户都有, 仅需两个线程, 一个收一个发
    thread sender(Send);
    thread receiver(Receive);

    sender.Start();
    receiver.Start();

    id = receiver.Id();

    sender.Join();
    receiver.Join();

    return 0;
}
