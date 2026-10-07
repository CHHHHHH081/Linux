#include "threadpool.hpp"
#include "TaskManager.hpp"

using namespace LogModule;
using namespace ThreadPoolModule;
using func_t = std::function<void()>;

int main()
{
    // std::string time = LogModule::GetTime();
    // std::cout << time << std::endl;
    // LOG(Loglevel::DEBUG) << "debug";

    TaskManager tm;
    int cnt = 10;
    while (cnt--)
    {
        threadpool<task_t>::GetInstance()->Enqueue(tm.GetRandomTask());
        sleep(1);
    }

    threadpool<task_t>::GetInstance()->Stop();
    sleep(1);
    threadpool<task_t>::GetInstance()->Join();

    return 0;
}