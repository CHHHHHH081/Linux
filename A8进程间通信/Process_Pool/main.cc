#include "ProcessPool.hpp"
int main()
{
    ProcessPool pp(5);
    pp.Create();
    int cnt = 10;
    while (cnt)
    {
        pp.Run();
        sleep(1);
        cnt--;
    }
    pp.Stop();
    return 0;
}