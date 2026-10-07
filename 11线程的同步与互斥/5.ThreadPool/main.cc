#include "threadpool.hpp"

using namespace LogModule;
using namespace ThreadPoolModule;
using func_t = std::function<void()>;

int main()
{
    std::string time = LogModule::GetTime();
    std::cout << time << std::endl;
    LOG(Loglevel::DEBUG) << "debug";

    // TaskManager tm;
    // threadpool<func_t> tp;
    // tp.Start();
    // sleep(3);

    return 0;
}