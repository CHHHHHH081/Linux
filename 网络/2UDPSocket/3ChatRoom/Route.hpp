#pragma once
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include "InetAddr.hpp"
#include "log.hpp"

using namespace LogModule;
using namespace Mutexmodule;

class Route
{
private:
    bool IsExist(InetAddr &user)
    {
        for (const auto &u : _online_users)
        {
            if (u == user)
                return true;
        }
        return false;
    }
    void AddUser(InetAddr &user)
    {
        _online_users.push_back(user);
        LOG(Loglevel::INFO) << "新增一个用户: " << user.AddrString();
    }
    void DeleteUser(InetAddr &user)
    {
        for (auto it = _online_users.begin(); it < _online_users.end(); it++)
        {
            if (*it == user)
            {
                _online_users.erase(it);
                LOG(Loglevel::INFO) << "删除一个用户: " << user.AddrString();
                break;
            }
        }
    }

public:
    Route()
    {
    }
    void MessageRoute(int sockfd, const std::string &msg, InetAddr &addr)
    {
        LockGuard lg(_mutex);
        if (!IsExist(addr))
            AddUser(addr);
        string send_msg = addr.AddrString() + "# " + msg;
        for (auto &user : _online_users)
        {
            int n = sendto(sockfd, send_msg.c_str(), send_msg.size(), 0, (const sockaddr *)&user.GetAddr(), sizeof(user.GetAddr()));
            if (n == -1)
            {
                LOG(Loglevel::ERROR) << "Route Send Error";
                continue;
            }
        }
        if (msg == "QUIT")
            DeleteUser(addr);
    }

private:
    std::vector<InetAddr> _online_users;
    Mutex _mutex;
};
