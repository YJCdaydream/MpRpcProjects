#pragma once
#include "google/protobuf/service.h"
#include <memory>
#include <functional>
#include <unordered_map>
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpConnection.h>
#include <google/protobuf/descriptor.h>


// 框架提供的专门服务发布RPC服务的网络对象类
class RpcProvider
{
private:
    // 组合了EventLoop
    muduo::net::EventLoop m_eventloop;

    // 服务类型信息
    struct ServiceInfo
    {
        google::protobuf::Service * m_service;     // 保存服务对象
        std::unordered_map<std::string, const google::protobuf::MethodDescriptor *> m_methodMap;    // 服务对象的方法以及名字
    };
    
    std::unordered_map<std::string, ServiceInfo> m_serviceMap;


    // 新的socket连接会回调
    void OnConnection(const muduo::net::TcpConnectionPtr&);
    // 已建立连接用户的读写事件回调
    void OnMessage(const muduo::net::TcpConnectionPtr&, muduo::net::Buffer*, muduo::Timestamp);

    // closure回调操作，用于序列化rpc的响应和网络发送, 参数为一个tcp连接，以及服务结束产生的response
    void SendRpcResponse(const muduo::net::TcpConnectionPtr&, google::protobuf::Message *);

public:
    // 框架提供给外部使用的，可以发布rpc服务
    void NotifyService(google::protobuf::Service *service);


    // 启动rpc服务节点
    void run();
};
