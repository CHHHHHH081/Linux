#include "progress.h"
#define NUM 101
#define STYLE '='

void progress_v1()
{
    char buffer[NUM];
    memset(buffer,0,sizeof(buffer));
    const char* label="|/-\\";
    int len=strlen(label);
    int cnt=0;
    while(cnt<=100)
    {
        buffer[cnt]=STYLE;
        printf("[%-100s][%d%%][%c]\r",buffer,cnt,label[cnt%len]);
        fflush(stdout);
        usleep(10000);
        cnt++;
    }
    printf("\n");
}
