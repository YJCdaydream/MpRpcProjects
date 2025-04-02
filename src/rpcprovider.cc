#include "rpcprovider.h"
#include "mprpcapplication.h"
#include "rpcheader.pb.h"
#include "logger.h"
#include "zookeeperutil.h"
#include <functional>

// 框架提供给外部使用的，可以发布rpc服务 
// 注册服务方以及服务方的函数
void RpcProvider::NotifyService(google::protobuf::Service * service)
{
    ServiceInfo service_info;
    const google::protobuf::ServiceDescriptor * pserviceDesc =  service ->GetDescriptor();
    std::string service_name = pserviceDesc->name();
    int methodCnt = pserviceDesc->method_count();

    
    //std::cout << "service name: " << service_name << std::endl << " method: ";
    for(int i = 0; i < methodCnt; i++)
    {
        const google::protobuf::MethodDescriptor * pmethodDesc =  pserviceDesc->method(i);
        std::string methodNam = pmethodDesc->name();

        // 插入全部的服务方法和对应名字
        service_info.m_methodMap.insert({methodNam, pmethodDesc});  
        LOG_INFO("service name: %s method:", service_name, methodNam);
        //std::cout << methodNam << " ";
    }

    // 存入服务对象
    service_info.m_service = service;   

    // 用服务名字，对应出服务的全部信息
    m_serviceMap.insert({service_name, service_info});      

};

// 启动rpc服务节点，epoll（多线程 服务器
void RpcProvider::run()
{
    std::string ip = MprpcApplication::GetInstance().GetMprpcconfig().Load("rpcserverip");
    uint16_t port = atoi(MprpcApplication::GetInstance().GetMprpcconfig().Load("rpcserverport").c_str());
    muduo::net::InetAddress address(ip, port);

    // 创建TCPServer对象
    muduo::net::TcpServer server(&m_eventloop, address, "RpcProvider");

    // 绑定连接回调和消息读写回调方法
    server.setConnectionCallback(std::bind(&RpcProvider::OnConnection, this, std::placeholders::_1));
    server.setMessageCallback(std::bind(&RpcProvider::OnMessage, this, std::placeholders::_1, 
                                        std::placeholders::_2, std::placeholders::_3));

    // 设置muduo库的线程数量
    server.setThreadNum(4);

    // 把当前rpc节点上要发布的服务全部注册到zk上面，让rpc client可以从zk上发现服务
    ZkClient zkClie;
    zkClie.Start();
    // service_name 为永久性节点  method_name为临时节点
    for(auto & sp : m_serviceMap)
    {
        // service_name 
        std::string service_path = "/" + sp.first;
        zkClie.Create(service_path.c_str(), nullptr, 0);
        for(auto &mp : sp.second.m_methodMap)
        {
            std::string method_path = service_path + "/" + mp.first;
            char method_path_data[128] = {0};
            sprintf(method_path_data, "%s:%d", ip.c_str(), port);
            zkClie.Create(method_path.c_str(), method_path_data, strlen(method_path_data), ZOO_EPHEMERAL);
        }
    }




    std::cout << "RpcProvider start service at ip:" << ip << "::" << port << std::endl;
    // 启动网络服务
    server.start();
    m_eventloop.loop();
};


// 新的socket连接回调
void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr& conn)
{
    if(!conn->connected())  // 说明连接断开了
    {
        conn->shutdown();   // 则直关闭连接
    }
}


/*
消息格式 head_size {service_name method_name args_size} args
        4 字节    

已建立连接用户的读写事件回调 如果远程有一个rpc服务的调用请求， 那么OnMessage方法就会响应*/
void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr& conn, 
                            muduo::net::Buffer* buffer, 
                            muduo::Timestamp)
{
    std::string recv_buf = buffer->retrieveAllAsString();  // 将接收到的所有字符转成string
    uint32_t header_size = 0;
    // 取收到字符流的前四个字节作为判断头的长度
    recv_buf.copy((char *)&header_size, 4, 0);
    
    std::string rpc_header_str = recv_buf.substr(4, header_size);

    mprpc::RpcHeader rpcHeader;
    std::string service_name;
    std::string method_name;
    uint32_t args_size;
    if(rpcHeader.ParseFromString(rpc_header_str))   // 反序列化成功
    {
        service_name = rpcHeader.service_name();
        method_name = rpcHeader.method_name();
        args_size = rpcHeader.args_size(); 
    }
    else
    {
        // 数据头反序列化失败
        std::cout << "rpc_header_str" << rpc_header_str << " parse error!" << std::endl;
        return;
    }
    
    // 获取参数的str
    std::string args_str = recv_buf.substr(4 + header_size, args_size);
    
    std::cout <<"============================================"<< std::endl;
    std::cout <<"header_size:"<< header_size << std::endl;
    std::cout <<"rpc_header_str:"<< rpc_header_str << std::endl;
    std::cout <<"service_name:"<< service_name << std::endl;
    std::cout <<"method_name:"<< method_name << std::endl;
    std::cout <<"args_str:"<< args_str << std::endl;
    std::cout <<"==========================================="<<std::endl;

    // 获取service对象和method方法
    auto it = m_serviceMap.find(service_name);
    if(it == m_serviceMap.end())   
    {
        // 说明该服务未注册 则直接退出
        std::cout << service_name << "is not exist!" << std::endl;
        return;
    }

    auto mit = it->second.m_methodMap.find(method_name);  // 该service下的method方法
    if(mit == it->second.m_methodMap.end())
    {
        // 说明该方法未注册 则直接退出
        std::cout << method_name << "is not exist!" << std::endl;
        return;
    } 

    google::protobuf::Service *service =  it->second.m_service;   // 获取service对象
    const google::protobuf::MethodDescriptor * method = mit->second;    // 获取method对象

    // 生成rpc的request 和response 对象, 都是抽象的基类，从而避免与业务逻辑耦合, 从service 对象的method中获取到对应的request 以及response
    google::protobuf::Message  *request = service->GetRequestPrototype(method).New();
    // args_str 反序列化得出调用方法的参数
    if(!request->ParseFromString(args_str))
    {
        std::cout << "request pase error! " << std::endl;
        return;
    }
    google::protobuf::Message *response = service->GetResponsePrototype(method).New();

    // 给下面的method方法的调用，绑定一个closure的回调函数
    google::protobuf::Closure* closure = google::protobuf::NewCallback<RpcProvider, 
                                                                       const muduo::net::TcpConnectionPtr&,
                                                                       google::protobuf::Message *>(this, &RpcProvider::SendRpcResponse, conn, response);


    // 得到了service method request response 则可以调用函数了,执行完后，则会调用closur中绑定的函数
    service->CallMethod(method, nullptr, request, response, closure);
}   


// closure回调操作，用于序列化rpc的响应和网络发送
void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr& conn, google::protobuf::Message * response)
{
    // 序列化response到response_str
    std::string response_str;
    if(response->SerializeToString(&response_str))
    {
        // 序列化成功后，则通过网络把rpc方法执行的结果发送回rpc的调用方
        conn->send(response_str);
        conn->shutdown(); // 模拟http的短链接服务，由rpcprovider主动断开连接
    }
    else
    {
        std::cout << "serialize response_str error" << std::endl;
    }
    conn->shutdown();
}