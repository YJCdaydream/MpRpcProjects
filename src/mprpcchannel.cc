#include "mprpcchannel.h"
#include "mprpcapplication.h"
#include "rpcheader.pb.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
 #include <unistd.h>


/*
消息格式 head_size {service_name method_name args_size} args
        4 字节
调用rpc consumer 服务的方法 则会触发该回调
*/
void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor *method,
                              google::protobuf::RpcController *controller,
                              const google::protobuf::Message *request,
                              google::protobuf::Message *response,
                              google::protobuf::Closure *done)
{
    const google::protobuf::ServiceDescriptor *serverDesc = method->service();
    std::string server_name = serverDesc->name();
    std::string method_name = method->name();
    uint32_t argc_size;

    std::string args_str;

    if (request->SerializeToString(&args_str))
    {
        argc_size = args_str.size();
    }
    else
    {
        controller->SetFailed("Serialize args_str failed");
        return ;
    }

    mprpc::RpcHeader rpcHeader;
    rpcHeader.set_service_name(server_name);
    rpcHeader.set_method_name(method_name);
    rpcHeader.set_args_size(argc_size);

    uint32_t header_size;
    std::string rpc_header_str;
    if (rpcHeader.SerializeToString(&rpc_header_str))
    {
        // 没有问题
        header_size = rpc_header_str.size();
    }
    else
    {
        controller->SetFailed("Serialize header failed");
        return;
    }

    // 将 header_size 加上 header 加上 参数
    std::string send_rpc_str;
    send_rpc_str.insert(0, (char *)&header_size, 4); // 在头部插入4个字节的head_size
    // 加上头和参数
    send_rpc_str += rpc_header_str;
    send_rpc_str += args_str;

    // 打印调试信息
    std::cout << "======================================================" << std::endl;
    std::cout << "header_size:" << header_size << std::endl;
    std::cout << "rpc_header_str:" << rpc_header_str << std::endl;
    std::cout << "service_name:" << server_name << std::endl;
    std::cout << "method_name:" << method_name << std::endl;
    std::cout << "args_str:" << args_str << std::endl;
    std::cout << "======================================================" << std::endl;

    // 使用tcp编程， 完成rpc远程调用 创建一个socket 
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == clientfd)
    {
        char errtxt[512] = {0};
        sprintf(errtxt, "create socket error! errno: %d", errno);
        controller->SetFailed(errtxt);
        exit(EXIT_FAILURE);
    }

    // 1. 事先配置好的服务节点
    std::string ip = MprpcApplication::GetInstance().GetMprpcconfig().Load("rpcserverip");
    uint16_t port = atoi(MprpcApplication::GetInstance().GetMprpcconfig().Load("rpcserverport").c_str());

    // 2. 查询Zookeeper获取ip和port
    {
        
    }

    // 写配置连接到服务器，采用同步阻塞的方法
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;   // ipv4
    server_addr.sin_port = htons(port); // 设置端口
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());    // 设置ip地址
    
    if(-1 == connect(clientfd, (struct sockaddr *)& server_addr, sizeof(server_addr)))
    {
        // 连接失败直接直接退出
        char errtxt[512] = {0};
        sprintf(errtxt, "connect error! errno: %d", errno);
        controller->SetFailed(errtxt);
        exit(EXIT_FAILURE);
    }

    
    // 发送rpc请求
    if(-1 == send(clientfd, send_rpc_str.c_str(), send_rpc_str.size(), 0))
    {
        // 发送失败
        char errtxt[512] = {0};
        sprintf(errtxt, "send error! errno: %d", errno);
        controller->SetFailed(errtxt);
        close(clientfd);
        return;
    }

    char recv_buf[1024] = {0};   // 设置一个1k的接受rpc缓存
    int recv_size = 0;
    // 接收rpc回应
    if(-1 == (recv_size = recv(clientfd, recv_buf, 1024, 0)))
    {
        // 接收失败
        char errtxt[512] = {0};
        sprintf(errtxt, "recv error! errno: %d", errno);
        controller->SetFailed(errtxt);
        close(clientfd);
        return ;
    }

    // 反序列化rpc调用的响应数据
    // std::string response_str(recv_buf, recv_size);
    if(!response->ParseFromArray(recv_buf, recv_size))
    {
        // 反序列化失败
        char errtxt[512] = {0};
        sprintf(errtxt, "parse error! response_str: %s", recv_buf);
        controller->SetFailed(errtxt);
        close(clientfd);
        return;
    }

    close(clientfd);
}