#ifndef __LOG_HPP
#define __LOG_HPP

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <unistd.h>
#include <memory>
#include <time.h>
#include <cstdio>
#include "Mutex.hpp"

namespace LogModule
{
#define GSEP "\r\n"

    namespace fs = std::filesystem;
    using namespace Mutexmodule;

    class LogStretage
    {
    public:
        virtual void SyncLog(const std::string &message) = 0;
        ~LogStretage() = default;
    };

    // 向显示器打印策略
    class ConsoleLogStretage : public LogStretage
    {
    public:
        void SyncLog(const std::string &message) override
        {
            LockGuard _lockguard(_mutex);
            std::cout << message << GSEP;
        }

    private:
        Mutex _mutex;
    };

    const std::string &defaultpath = "./log/";
    const std::string &defaultfile = "my.log";
    // 向文件打印策略
    class FileLogStretage : public LogStretage
    {
    public:
        FileLogStretage(const std::string &path = defaultpath, const std::string &file = defaultfile)
            : _path(path), _file(file)
        {
            if (fs::exists(_path))
            {
                return;
            }
            fs::create_directories(path);
        }

        void SyncLog(const std::string &message) override
        {
            LockGuard _lockguard(_mutex);
            std::string filename = (_path.back() == '/' ? _path : (_path + '/')) + _file;
            std::ofstream ofs(filename, std::ios::app);
            if (!ofs)
            {
                std::cerr << "打开文件失败\n"
                          << std::endl;
                return;
            }
            ofs << message << GSEP;
            ofs.close();
        }

    private:
        std::string _path;
        std::string _file;
        Mutex _mutex;
    };

    std::string GetTime()
    {
        time_t t = time(nullptr);
        struct tm tm;
        localtime_r(&t, &tm);
        char timebuf[128];
        snprintf(timebuf, sizeof(timebuf), "%d-%d-%d %02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
        return timebuf;
    }

    enum class Loglevel
    {
        DEBUG,
        NORMAL,
        WARNING,
        ERROR,
        FATAL
    };

    std::string LevelToStr(Loglevel &lev)
    {
        switch (lev)
        {
        case Loglevel::DEBUG:
            return "DEBUG";
        case Loglevel::NORMAL:
            return "NORMAL";
        case Loglevel::WARNING:
            return "WARNING";
        case Loglevel::ERROR:
            return "ERROR";
        case Loglevel::FATAL:
            return "FATAL";
        default:
            return "UNKNOWN";
        }
    }

    class Logger
    {
    public:
        void EnableConsoleStretage()
        {
            _fflush_stretage = std::make_unique<ConsoleLogStretage>();
        }

        void EnableFileStretage()
        {
            _fflush_stretage = std::make_unique<FileLogStretage>();
        }

        Logger()
        {
            EnableFileStretage();
        }

        class LoggerMessage
        {
        public:
            LoggerMessage(Loglevel lev, std::string &file, int line_number, Logger &logger)
                : _time(GetTime()), _lev(lev), _pid(getpid()), _file(file), _line_number(line_number), _logger(logger)
            {
                std::stringstream ss;
                ss << "[" << _time << "]" << "[" << LevelToStr(lev) << "]" << "[" << _pid << "]" << "[" << _file << "]" << "[" << _line_number << "] - ";
                _loginfo = ss.str();
            }

            template <typename T>
            LoggerMessage &operator<<(const T &msg)
            {
                std::stringstream ss;
                ss << msg;
                _loginfo += ss.str();
                return *this;
            }

            ~LoggerMessage()
            {
                if (_logger._fflush_stretage)
                {
                    _logger._fflush_stretage->SyncLog(_loginfo);
                }
            }

        private:
            Loglevel _lev;        // 日志等级
            std::string _file;    // 发出日志源文件
            std::string _time;    // 时间
            pid_t _pid;           // 进程pid
            int _line_number;     // 行数
            std::string _loginfo; // 合并信息   [2026-9-28 18:48:27][level][pid][filename][line_number] - XXXXX
            Logger &_logger;
        };

        LoggerMessage operator()(Loglevel lev, std::string file, int line)
        {
            return LoggerMessage(lev, file, line, *this);
        }

    public:
        std::unique_ptr<LogStretage> _fflush_stretage;
    };

    // 全局日志对象
    Logger logger;

#define LOG(level) logger(level, __FILE__, __LINE__)
#define Enable_Console_Stretage() logger.EnableConsoleStretage()
#define Enable_File_Stretage() logger.EnableFileStretage()
}

#endif
