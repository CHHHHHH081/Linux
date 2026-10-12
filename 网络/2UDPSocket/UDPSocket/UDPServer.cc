#include "UDPServer.hpp"

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
    unique_ptr<UDPSocket> usvr = make_unique<UDPSocket>(port, defaulthandler);
    usvr->Init();
    usvr->Start();
    return 0;
}