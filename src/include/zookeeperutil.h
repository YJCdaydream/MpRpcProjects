#pragma once
#include <zookeeper/zookeeper.h>
#include <string>

class ZkClient
{
private:
    // zk的客户端句柄
    zhandle_t * m_zhandle;
public:
    ZkClient(/* args */);
    ~ZkClient();

    // zkclient 启动连接zkserver
    void Start();

    // 在zkserver上根据指定的path创建一个znode节点
    void Create(const char * path, const char * data, int datalen, int state = 0);

    // 根据参数指定的znode节点路径，获取znode节点的值
    std::string GetData(const char * path);
};


