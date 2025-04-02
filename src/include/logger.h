#pragma once 

#include "lockqueue.h"


enum LogLevel
{
    INFO,      // 普通信息
    ERROR,   // 错误信息
};

// Mprpc框架提供的log系统
class Logger
{
private:
    /* data */
    int m_loglevel;         // 记录日志级别
    LockQueue<std::string> m_lckQue;       // 日志缓冲队列
    Logger();

    Logger(Logger &) = delete;  // 封印住所有的拷贝构造函数
    Logger(Logger &&) = delete;    // 右值移动构造
public:
    static Logger& getInstance();   // 获取对象实例

    void SetLogLevel(LogLevel level);
    void Log(std::string msg);

};




// 使用宏定义使得用户调用变得方便
#define LOG_INFO(logmsformat, ...)  \
        do  \
        {   \
            Logger& logger = Logger::getInstance();\
            logger.SetLogLevel(INFO); \
            char c[1024] = {0};\
            snprintf(c, 1024, logmsformat, ##__VA_ARGS__);\
            logger.Log(c);\
        } while (0);

// 错误日志的宏定义 //每次创建一个对象就把上一次的日志写入到了日志文件里面 // 写入到日志队列中，还没有写入到文件里
#define LOG_ERR(logmsformat, ...)  \
        do  \
        {   \
            Logger& logger = Logger::getInstance();  \
            logger.SetLogLevel(ERROR); \
            char c[1024] = {0}; \
            snprintf(c, 1024, logmsformat, ##__VA_ARGS__);\
            logger.Log(c);  \
        } while (0);
        