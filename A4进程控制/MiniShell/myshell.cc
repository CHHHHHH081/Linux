#include <unistd.h>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <cstring>
#include <sys/wait.h>
using namespace std;

//1.命令行参数及个数
const int basenum=1024;
const int argvnum=64;
char *gargv[argvnum];
int gargc=0;

//2.环境变量表
const int envnum=64;
char *genv[envnum];
int genvs=0;

//当前工作目录
const int cwdlen=1024;
char cwd[cwdlen];

//上一次退出码
int lastcode=0;

string GetUserName()
{
    const char* username=getenv("USER");
    return (username!=nullptr&&username[0]!='\0')?username:"none";
}

string GetLogName()
{
    const char* logname=getenv("LOGNAME");
    return (logname!=nullptr&&logname[0]!='\0')?logname:"none";
}

string GetPWD()
{
    const char* pwd=getcwd(cwd,sizeof(cwd));
    if(pwd!=NULL)
    {
        setenv("PWD",pwd,1);
    }
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

void cd()
{
    if(gargc==1||!strcmp(gargv[1],"~"))
    {
        //家目录
        if(chdir(getenv("HOME"))==-1)
            perror("cd");
    }
    else
    {
        if(chdir(gargv[1])==-1)
            perror("cd");
    }
}

void echo()
{
    if(gargc==1)
    {
        printf("\n");
        return;
    }
    char *ret=gargv[1];
    if(ret[0]=='$')
    {
        if(ret[1]==0)
            printf("$\n");
        else if(!strcmp(ret,"$?"))
            printf("%d\n",lastcode);
        else
        {
            char *en=getenv(&ret[1]);
            if(en==NULL)
                printf("\n");
            else
                printf("%s\n",en);
        }
    }
    else
        printf("%s\n",ret);
}

//4.1判断并执行内建命令
bool CheckAndExe()
{
    //输入为空，直接进入下一轮循环
    if(gargc==0) return true;
    char *cmd=gargv[0];
    if(!strcmp(cmd,"cd"))
    {
        cd();
        return true;
    }
    else if(!strcmp(cmd,"echo"))
    {
        echo();
        return true;
    }
    return false;
}

//4.2执行命令
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
        lastcode=WIFEXITED(status);
    }
    return false;
}

void InitEnv()
{
    extern char **environ;
    memset(genv,0,sizeof(genv));
    genvs=0;
    for(int i=0;environ[i];i++)
    {
        genv[i]=(char*)malloc(strlen(environ[i])+1);
        if(genv[i]==NULL)
            perror("malloc fail!");
        strcpy(genv[i],environ[i]);
        genvs++;
    }
    genv[genvs]=NULL;
    for(int i=0;genv[i];i++)
    {
        putenv(genv[i]);
    }
    environ=genv;
}

int main()
{
    InitEnv();
    char CmdBuffer[basenum];
    while(1)
    {
        //1.打印命令行提示符
        PrintCommandLine();
        //2.获取用户命令，如果没有输入则继续下一行
        if(!GetCmd(CmdBuffer,basenum))
            continue;
        //用户命令存在CmdBuffer中
        //3.分析用户命令
        ParseCmdLine(CmdBuffer);
        //命令行参数存在gargv中
        //4.1判断是否为内建命令，是就执行然后下一轮循环
        //不是就为普通命令
        if(CheckAndExe())
            continue;
        //4.2执行命令
        ExecuteCmd();
    }
    return 0;
}
