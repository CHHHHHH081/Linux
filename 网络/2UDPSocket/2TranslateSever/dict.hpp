#pragma once
#include <string>
#include <unordered_map>
#include <filesystem>
#include <iostream>
#include <fstream>
#include "InetAddr.hpp"
#include "log.hpp"

using namespace LogModule;
using namespace std;

const string dictpath = "./dictionary.txt";
const string sep = ": ";

class dict
{
public:
    dict(const string path = dictpath)
        : _path(path)
    {
    }
    void LoadDict()
    {
        ifstream in(_path);
        if (!in.is_open())
        {
            LOG(Loglevel::ERROR) << "Open File Error";
            exit(1);
        }
        string line;
        while (getline(in, line))
        {
            int pos = line.find(sep);
            if (pos == string::npos)
            {
                LOG(Loglevel::WARNING) << "解析: " << line << " 失败";
                continue;
            }
            // 解析成功(当行存在sep)
            string English = line.substr(0, pos);
            string Chinese = line.substr(pos + sep.size(), string::npos);
            // 判断sep前后是否为空
            if (English.size() == 0 || Chinese.size() == 0)
            {
                LOG(Loglevel::WARNING) << line << " 没有有效内容";
                continue;
            }
            // sep前后不空, 当行信息正常, 录入哈希表
            _dicmap[English] = Chinese;
        }
        in.close();
    }

    string Translate(string eng, InetAddr &addr)
    {
        auto it = _dicmap.find(eng);
        if (it == _dicmap.end())
        {
            LOG(Loglevel::NORMAL) << "[" << addr.AddrString() << "]进入翻译: " << eng << " -> NONE";
            return "NONE";
        }
        else
        {
            string ret = it->second;
            LOG(Loglevel::NORMAL) << "[" << addr.AddrString() << "]进入翻译: " << eng << " -> " << ret;
            return ret;
        }
    }

private:
    string _path;
    unordered_map<string, string> _dicmap;
};
