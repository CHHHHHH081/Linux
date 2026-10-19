#include "TCPServer.hpp"
#include "Command.hpp"
#include <memory>

void Usage()
{
    std::cerr << "usage err" << std::endl;
    exit(USAGE_ERR);
}

void defaultfunc(void)
{
}

int main(int argc, char *argv[])
{
    Enable_Console_Stretage();
    if (argc != 2)
        Usage();
    uint16_t port = std::stoi(argv[1]);
    Command Cmds;
    std::unique_ptr<TCPServer> tcp = std::make_unique<TCPServer>(port, [&Cmds](std::string cmd, InetAddr peer) -> std::string
                                                                 { return Cmds.Execute(cmd, peer); });
    tcp->Init();
    tcp->Start();
    return 0;
}
