#include "include/logger.h"
#include "time.h"
#include <iostream>

Logger& Logger::getInstance()
{
    static Logger logger;
    return logger;
}


Logger::Logger()
{
    // 启动专门的写日志线程
    std::thread writeLogTask([&](){
        for(;;)
        {
            // 获取当前时间
            time_t now = time(nullptr);
            tm * nowt = localtime(&now); int tm_mday;	
            
            // 设置当前时间为文件名
            char file_name[128];
            sprintf(file_name, "%d-%d-%d-log.txt", nowt->tm_year + 1900, nowt->tm_mon + 1, nowt->tm_mday);
            FILE * pf = fopen(file_name, "a+");
            if(pf == nullptr)
            {
                // 打开文件失败
                std::cout << "open file failed!" << std::endl;
                exit(EXIT_FAILURE);
            }

            // 写入到文件中
            std::string msg = m_lckQue.Pop();

            // 插一个具体时间到行开头,以及末尾插一个换行
            char time_buf[128];
            sprintf(time_buf, "%d:%d:%d => [%s]", nowt->tm_hour, nowt->tm_min, nowt->tm_sec, (m_loglevel == INFO ? "INFO" : "ERR"));
            msg.insert(0, time_buf);
            msg.append("\n");

            fputs(msg.c_str(), pf);

            fclose(pf);
        }
    });
    // 设置分离线程，守护线程,主线不管该线程的执行结果了
    writeLogTask.detach();
}

void Logger::SetLogLevel(LogLevel level)
{
    m_loglevel = level;
}

void Logger::Log(std::string msg)
{
    m_lckQue.Push(msg);
}