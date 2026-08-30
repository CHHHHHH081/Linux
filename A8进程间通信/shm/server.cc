#include "shm.hpp"

int main()
{
    umask(0);
    shm shm;
    shm.Create();
    sleep(5);
    shm.Attach();
    sleep(5);
    return 0;
}