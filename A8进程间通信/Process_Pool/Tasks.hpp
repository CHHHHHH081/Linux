#pragma once
#include <iostream>
#include <ctime>
#include <functional>

void PrintLog() { std::cout << "打印日志..." << std::endl; }
void DownLoad() { std::cout << "下载内容..." << std::endl; }
void UpLoad() { std::cout << "上传内容..." << std::endl; }

typedef std::function<void(void)> task_t;

class TaskManager
{
public:
    TaskManager() { srand(time(nullptr)); }
    int Code() { return rand() % _tm.size(); }
    void Execute(int code) { _tm[code](); }
    void Register(task_t f) { _tm.push_back(f); }

private:
    std::vector<task_t> _tm;
};
