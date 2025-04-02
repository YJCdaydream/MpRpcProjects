#include "include/zookeeperutil.h"
#include "include/mprpcapplication.h"   // 获取配置
#include <semaphore.h>
#include <iostream>

// watch callback 函数  zkserver给zkclient的通知
void global_watch(zhandle_t *zh, int type, int state, const char *path,void *watcherCtx)
{
    if(type == ZOO_SESSION_EVENT)   // 回调的消息类型是和会话相关的消息类型
    {
        if(state == ZOO_CONNECTED_STATE)
        {
            // 说明连接成功了，增加信号量，使得同步进程能接着往下运行

            sem_t *sem = (sem_t*) zoo_get_context(zh);
            sem_post(sem);  // 增加信号量值，解除阻塞
        }
    }

}

ZkClient::ZkClient(/* args */) : m_zhandle(nullptr)
{
}

ZkClient::~ZkClient()
{
    // 说明连接成功了，则断开连接
    if(m_zhandle != nullptr)
    {
        zookeeper_close(m_zhandle);
    }
}

void ZkClient::Start()
{
    // 创建连接
    std::string host = MprpcApplication::GetInstance().GetMprpcconfig().Load("zookeeperip");
    std::string port = MprpcApplication::GetInstance().GetMprpcconfig().Load("zookeeperport");
    std::string connstr = host + ":" + port;

    // 异步创建句柄
    /*
        zookeeper_mt: 多线程版本
        zookeeper的API客户端程序提供了三个线程
        API调用线程
        网络I/O线程 pthread_create => poll
        watcher回调线程 连接成功则会执行watcher回调
    */
    m_zhandle = zookeeper_init(connstr.c_str(), global_watch, 30000, nullptr, nullptr, 0);
    if(nullptr == m_zhandle)
    {
        // 句柄创建未成功，则应该退出
        exit(EXIT_FAILURE);
    }

    // 创建一个信号量，来同步zookeeper连接，等到连接成功再执行其他业务
    sem_t sem;
    sem_init(&sem, 0, 0);
    zoo_set_context(m_zhandle, &sem);

    sem_wait(&sem);
    std::cout<< "zookeeper init success" << std::endl;
}

void ZkClient::Create(const char * path, const char * data, int datalen, int state)
{
    char path_buffer[128];
    int buffer_len = sizeof(path_buffer);
    int flag;
    flag = zoo_exists(m_zhandle, path, 0, nullptr);

    if(ZNONODE == flag)
    {
        flag = zoo_create(m_zhandle, path, data, datalen,
                          &ZOO_OPEN_ACL_UNSAFE, state, path_buffer, buffer_len);
        if(flag == ZOK)
        {
            // 创建成功
            std::cout << "znode create success... path:" << path << std::endl;
        }
        else
        {
            // 创建失败
            std::cout << "flag:" << flag << std::endl;
            std::cout << "znode create error... path:" << path << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}

// 根据相应的path 获取相应znode的值
std::string ZkClient::GetData(const char * path)
{
    char buffer[64];
    int buffer_len = sizeof(buffer);
    int flag = zoo_get(m_zhandle, path , 0, buffer, &buffer_len, nullptr);
    if(flag != ZOK)
    {
        std::cout << "get znode error... path:" << path << std::endl;
        return "";
    }
    else
    {
        return buffer;
    }
}