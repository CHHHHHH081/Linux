#include "UDPServer.hpp"
#include "dict.hpp"
#include "InetAddr.hpp"

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
    dict dict;
    dict.LoadDict();
    unique_ptr<UDPSocket> usvr = make_unique<UDPSocket>(port, [&dict](const string eng, InetAddr &addr) -> string
                                                        { return dict.Translate(eng, addr); });
    usvr->Init();
    usvr->Start();
    return 0;
}