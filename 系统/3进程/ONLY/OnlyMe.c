#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc,char* argv[],char* env[])
{
    (void)argc;
    (void)argv;
    (void)env;
    
    char* who=getenv("USER");
    if(who==NULL)
        return 1;
    if(strcmp(who,"chh")==0)
        printf("正常\n");
    else
        printf("错误\n");
    return 0;
}

