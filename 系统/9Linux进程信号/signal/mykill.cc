#include <sys/types.h>
#include <signal.h>
#include <iostream>
#include <string>

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cout << "Use Error!" << std::endl;
        return 1;
    }
    int signal = std::stoi(argv[1]);
    pid_t pid = std::stoi(argv[2]);
    int n = kill(pid, signal);
    if (n == 0)
    {
        std::cout << "Send Signal " << signal << "To [" << pid << "]" << std::endl;
    }
    else if (n == -1)
    {
        std::cout << "Send Fail!" << std::endl;
        return 1;
    }
    return 0;
}