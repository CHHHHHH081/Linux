#pragma once
#include <unordered_set>
#include <iostream>
#include <string>
#include "InetAddr.hpp"

class Command
{
public:
    Command()
        : _EnableWhiteList(true)
    {
        _WhiteCmdList.insert("ls");
        _WhiteCmdList.insert("ls -l");
        _WhiteCmdList.insert("ls -al");
        _WhiteCmdList.insert("ll");
        _WhiteCmdList.insert("pwd");
        _WhiteCmdList.insert("whoami");
        _WhiteCmdList.insert("touch haha.txt");
    }
    void UnEnableWhiteList() { _EnableWhiteList = false; }
    void EnableWhiteList() { _EnableWhiteList = true; }
    bool IsSafeCmd(std::string cmd) { return _WhiteCmdList.find(cmd) != _WhiteCmdList.end(); }
    std::string Execute(std::string cmd, InetAddr addr)
    {
        // 检查是否启用白名单及命令是否为白名单命令
        if (_EnableWhiteList)
        {
            if (!IsSafeCmd(cmd))
            {
                return std::string("别乱搞");
            }
        }
        std::string who = addr.AddrString();

        // 开始执行
        FILE *fp = popen(cmd.c_str(), "r");
        if (nullptr == fp)
        {
            return std::string("根本没有这样的命令: ") + cmd;
        }
        std::string res;
        char line[1024];
        while (fgets(line, sizeof(line), fp))
        {
            res += line;
        }
        std::string ret = "成功, 结果: \n" + res;
        return ret;
    }

private:
    std::unordered_set<std::string> _WhiteCmdList;
    bool _EnableWhiteList;
};
