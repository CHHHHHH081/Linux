#include "log.hpp"

using namespace LogModule;
int main()
{
    // std::string time = LogModule::GetTime();
    // std::cout << time << std::endl;
    LOG(Loglevel::DEBUG) << "debug";
    return 0;
}