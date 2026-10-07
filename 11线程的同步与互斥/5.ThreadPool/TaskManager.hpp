#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <functional>

void DownloadTask()
{
    std::cout << "这是一个下载的任务......" << std::endl;
}

void UploadTask()
{
    std::cout << "这是一个上传的任务......" << std::endl;
}

void SQLTask()
{
    std::cout << "这是一个关于SQL的任务......" << std::endl;
}

using task_t = std::function<void(void)>;

class TaskManager
{
public:
    TaskManager()
    {
        _tasks.push_back(DownloadTask);
        _tasks.push_back(UploadTask);
        _tasks.push_back(SQLTask);
    }

    task_t GetTask(unsigned int index) { return _tasks[index]; }

    unsigned int Tasknum() { return _tasks.size(); }

private:
    std::vector<task_t> _tasks;
};
