#include "shm.hpp"

int main()
{
    shm shm;
    shm.Get();
    sleep(5);
    shm.Attach();
    sleep(5);
    return 0;
}
