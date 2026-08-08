#include <unistd.h>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <cstring>
#include <sys/wait.h>
using namespace std;

const int basenum=1024;
const int argvnum=64;
char *gargv[argvnum];
int gargc=0;
//退出码
int lastcode=0;

string GetUserName()
{
    char* username=getenv("USER");
    return (username!=nullptr&&username[0]!='\0')?username:"none";
}

string GetLogName()
{
    char* logname=getenv("LOGNAME");
    return (logname!=nullptr&&logname[0]!='\0')?logname:"none";
}

string GetPWD()
{
    char* pwd=getenv("PWD");
    return (pwd!=nullptr&&pwd[0]!='\0')?pwd:"none";
}

string MakeCommandLine()
{
   char cmdline[basenum];
   snprintf(cmdline,basenum,"%s@%s:%s$ ",GetUserName().c_str(),GetLogName().c_str(),GetPWD().c_str());
   return cmdline;
}

//1.命令行提示符
void PrintCommandLine()
{
    printf("%s",MakeCommandLine().c_str());
    fflush(stdout);
}

//2.获取用户命令
bool GetCmd(char *cmdbuffer,int size)
{
    char *result=fgets(cmdbuffer,size,stdin);
    if(!result)
        return false;
    cmdbuffer[strlen(cmdbuffer)-1]=0;
    if(strlen(cmdbuffer)==0)
        return false;
    return true;
}

//3.分析命令
void ParseCmdLine(char* CmdBuffer)
{
    memset(gargv,0,sizeof(gargv));
    gargc=0;
    const char *sep=" ";
    gargv[gargc++]=strtok(CmdBuffer,sep);
    while((bool)(gargv[gargc++]=strtok(nullptr,sep)));
    gargc--;
}

void test()
{
    printf("argc:%d\n",gargc);
    for(int i=0;gargv[i];i++)
    {
        printf("argv[%d]:%s\n",i,gargv[i]);
    }
}

bool ExecuteCmd()
{
    pid_t id=fork();
    if(id<0) return false;
    if(id==0)
    {
        //子进程
        execvp(gargv[0],gargv);
        exit(1);
    }
    int status=0;
    pid_t rid=waitpid(id,&status,0);
    if(rid>0)
    {
        if(WIFEXITED(status))
        {
            lastcode=WEXITSTATUS(status);
        }
        else
        {
            rid=100;
        }
        return true;
    }
    return false;
}

int main()
{
    char CmdBuffer[basenum];
    while(1)
    {
        PrintCommandLine();
        if(!GetCmd(CmdBuffer,basenum))
            continue;
        ParseCmdLine(CmdBuffer);
        ExecuteCmd();
    }
    return 0;
}
