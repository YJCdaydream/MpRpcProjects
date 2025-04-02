#include <iostream>
#include "mprpcapplication.h"
#include "../user.pb.h"


int main(int argc, char **argv)
{
    MprpcApplication::Init(argc, argv);

    fixbug::UserServiceRpc_Stub stub(new MprpcChannel());
    
    // rpc方法的请求参数
    fixbug::LoginRequest request;
    request.set_name("zhang san");
    request.set_pwd("123456");

    // 创建一个control对象控制是否请求成功
    MprpcController controller;

    fixbug::LoginResponse response;
    stub.Login(&controller, &request, &response, nullptr);

    if(controller.Failed())
    {
        std::cout << controller.ErrorText() << std::endl;
    }
    else
    {
        //一次rpc调用完成，读调用的结果
        if (0 == response.res().errcode())
        {
            std::cout << "rpc login response success:" << response.success() << std::endl;
        }
        else
        {
            std::cout << "rpc login response error: " << response.res().errmessage() << std::endl;
        }
    }

    return 0;
}




 //  执行注册操作
void registerFunction(fixbug::UserServiceRpc_Stub stub)
{
    fixbug::RegisterRequest registReq;
    fixbug::RegisterRepose registRes;

    // 设置请求头
    registReq.set_id(1);
    registReq.set_name("li si");
    registReq.set_pwd("8888888");
    stub.Register(nullptr, &registReq, &registRes, nullptr);
    
    if (0 == registRes.res().errcode())
    {
        std::cout << "rpc register response success:" << registRes.success() << std::endl;
    }
    else
    {
        std::cout << "rpc register response error: " << registRes.res().errmessage() << std::endl;
    }

}