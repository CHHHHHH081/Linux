#include "UDPServer.hpp"
#include "InetAddr.hpp"
#include "Route.hpp"
#include "threadpool.hpp"

using namespace ThreadPoolModule;
using task_t = function<void()>;

// 测试用函数
string defaulthandler(string s)
{
    string ret = "Hello, ";
    ret += s;
    return ret;
}

int main(int argc, char *argv[])
{
    Enable_Console_Stretage();
    if (argc != 2)
    {
        LOG(Loglevel::ERROR) << "Server Usage: " << argv[0] << " port";
        exit(1);
    }
    uint16_t port = stoi(argv[1]);
    auto tp = threadpool<task_t>::GetInstance();
    Route r;
    unique_ptr<UDPSocket> usvr = make_unique<UDPSocket>(port, [&r, &tp](int sockfd, const string &msg, InetAddr &addr)
                                                        {
        task_t task=std::bind(&Route::MessageRoute,&r,sockfd,msg,addr);
        tp->Enqueue(task); });
    usvr->Init();
    usvr->Start();
    return 0;
}