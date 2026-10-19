#pragma once
#include <unordered_set>
#include <iostream>
#include <string>
#include "InetAddr.hpp"

class Command
{
private:
    bool IsSafeCmd(std::string cmd) { return _WhiteCmdList.find(cmd) != _WhiteCmdList.end(); }
    static std::string Trim(const std::string &s)
    {
        auto b = s.find_first_not_of(" \t\r\n");
        auto e = s.find_last_not_of(" \t\r\n");
        return (b == std::string::npos) ? std::string() : s.substr(b, e - b + 1);
    }

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
    std::string Execute(std::string cmd, InetAddr addr)
    {
        cmd = Trim(cmd);
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
        FILE *fp = popen((cmd + " 2>&1").c_str(), "r");
        if (nullptr == fp)
        {
            return std::string("根本没有这样的命令: ") + cmd;
        }
        std::string res;
        char line[1024];
        const size_t MAX_OUT = 64 * 1024;
        while (fgets(line, sizeof(line), fp))
        {
            res += line;
            if (res.size() > MAX_OUT)
            {
                res += "\n...输出爆了";
                break;
            }
        }
        int n = pclose(fp);
        if (n == -1 || !(WIFEXITED(n) && WEXITSTATUS(n) == 0))
        {
            return who + "# 失败, 结果: \n" + res;
        }
        return who + "# 成功, 结果: \n" + res;
    }

private:
    std::unordered_set<std::string> _WhiteCmdList;
    bool _EnableWhiteList;
};
