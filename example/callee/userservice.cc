#include <iostream>
#include <string>
#include "user.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"
#include "logger.h"

/*
User
*/
class UserService : public fixbug::UserServiceRpc 
{
public:
    bool Login(std::string name, std::string pwd)
    {
        std::cout << "doing local service: Login" << std::endl;
        std::cout << "name: " << name << "pwd: " << pwd << std::endl;

        return true;
    }

    bool Register(uint32_t id, std::string name, std::string pwd)
    {
        std::cout << "doing local service: Login" << std::endl;
        std::cout <<"id: " << id << "name:" << name << "pwd: " << pwd << std::endl;

        return false;
    }


    // 重写基类UserServiceRPC的虚函数，下面都是框架提供的方法直接调用，并没有自己实现框架
    void Register(::google::protobuf::RpcController* controller,
        const ::fixbug::RegisterRequest* request,
        ::fixbug::RegisterRepose* response,
        ::google::protobuf::Closure* done)
    {
        // 获取请求的参数
        uint32_t id = request->id();
        std::string name = request->name();
        std::string pwd = request->pwd();

        // 调用本地方法，获取结果
        bool flag = Register(id, name, pwd);

        // 设置回应
        fixbug::Result* res = response->mutable_res();
        res->set_errcode(1);
        res->set_errmessage("register failed");
        response->set_success(flag);

        // 执行回调
        done->Run();
    }


    // 重写基类UserServiceRPC的虚函数，下面都是框架提供的方法直接调用，并没有自己实现框架
    void Login(::google::protobuf::RpcController *controller,
               const ::fixbug::LoginRequest *request,
               ::fixbug::LoginResponse *response,
               ::google::protobuf::Closure *done) override
    {
        // 远端发来的请求经过框架发送到该方法
        std::string name = request->name();
        std::string pwd = request->pwd();
        bool login_result = Login(name, pwd); // 本地业务

        // 把响应写入
        fixbug::Result *res = response->mutable_res();
        res->set_errcode(1);
        res->set_errmessage("login failed");
        response->set_success(login_result);

        // 执行回调操作发回给请求者
        done->Run();
    }
};

int main(int argc, char ** argv)
{
    LOG_INFO("第一条log message");
    LOG_ERR("%d:%d:%d", 22, 11, 41)
    // 调用框架的初始化操作
    MprpcApplication::Init(argc, argv);

    // 1.注册服务
    RpcProvider provide;
    provide.NotifyService(new UserService());   

    // 2.创建线程等待user调用（mudo）网络库，即开启服务
    provide.run();

    
    return 0;
}