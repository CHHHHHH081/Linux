#pragma once
#include <iostream>
#include <vector>
#include <unistd.h>
#include <string>
#include <cstdlib>
#include <sys/wait.h>
#include "Tasks.hpp"

class Channel
{
public:
    Channel(int wfd, pid_t subid)
        : _wfd(wfd), _subid(subid)
    {
        _name = "Channel-" + std::to_string(subid) + "-" + std::to_string(wfd);
    }
    ~Channel()
    {
    }

    void Send(int code)
    {
        ssize_t n = write(_wfd, &code, sizeof(code));
        (void)n;
    }

    int wfd() { return _wfd; }
    pid_t subid() { return _subid; }
    std::string name() { return _name; }

    // 关闭管道的写端
    void OnceCloseW() { close(_wfd); }
    // 关闭管道的子进程
    void OnceWait() { waitpid(_subid, nullptr, 0); }

private:
    int _wfd;
    pid_t _subid;
    std::string _name;
};

class ChannelManager
{
public:
    ChannelManager()
        : _next(0)
    {
    }
    ~ChannelManager()
    {
    }

    void Insert(int wfd, pid_t subid)
    {
        _channels.emplace_back(wfd, subid);
    }

    void PrintChannel()
    {
        for (auto &channel : _channels)
        {
            std::cout << channel.name() << std::endl;
        }
    }

    Channel &Select()
    {
        Channel &c = _channels[_next];
        _next++;
        _next %= _channels.size();
        return c;
    }

    // 关闭写端
    void CloseW()
    {
        for (auto &c : _channels)
            c.OnceCloseW();
    }

    // 回收子进程
    void Wait()
    {
        for (auto &c : _channels)
            c.OnceWait();
    }

private:
    std::vector<Channel> _channels;
    int _next;
};

// 默认管道数
const int gdefaultnum = 5;

class ProcessPool
{
public:
    ProcessPool(int num)
        : _process_num(num)
    {
        _tm.Register(PrintLog);
        _tm.Register(DownLoad);
        _tm.Register(UpLoad);
    }
    ~ProcessPool()
    {
    }

    void Work(int rfd)
    {
        int code = 0;
        while (1)
        {
            ssize_t n = read(rfd, &code, sizeof(code));
            if (n > 0)
            {
                if (n != sizeof(code))
                    continue;
                std::cout << "子进程[" << getpid() << "]得到一个任务码：" << code << std::endl;
                _tm.Execute(code);
            }
            else if (n == 0)
            {
                std::cout << "子进程退出" << std::endl;
                break;
            }
            else
            {
                std::cout << "读取错误" << std::endl;
                break;
            }
        }
    }

    bool Create()
    {
        for (int i = 0; i < _process_num; i++)
        {
            int pipefd[2] = {0};
            int n = pipe(pipefd);
            if (n == -1)
                return false;
            pid_t subid = fork();
            if (subid < 0)
                return false;
            else if (subid == 0)
            {
                // 子进程，读
                close(pipefd[1]);
                _cm.CloseW();
                Work(pipefd[0]);
                close(pipefd[0]);
                exit(0);
            }
            else
            {
                // 父进程，写
                close(pipefd[0]);
                _cm.Insert(pipefd[1], subid);
            }
        }
        return true;
    }

    void Debug()
    {
        _cm.PrintChannel();
    }

    void Run()
    {
        Channel &c = _cm.Select();
        std::cout << "选取子进程：" << c.name() << std::endl;
        int code = _tm.Code();
        std::cout << "发送任务码：" << code << std::endl;
        c.Send(code);
    }

    void Stop()
    {
        _cm.CloseW();
        std::cout << "已关闭所有写入端" << std::endl;
        _cm.Wait();
        std::cout << "已回收所有子进程" << std::endl;
    }

private:
    ChannelManager _cm;
    int _process_num = 5;
    TaskManager _tm;
};
