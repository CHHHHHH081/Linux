#include <stdio.h>

int main(int argc,char* argv[],char* env[])
{
    (void)argc;
    (void)argv;
    int i=0;
    for(;env[i];i++)
    {
        printf("env[%d]->%s\n",i,env[i]);
    }
    return 0;
}
