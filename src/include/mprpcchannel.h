#pragma once
#include<google/protobuf/service.h>
#include<google/protobuf/message.h>
#include<google/protobuf/descriptor.h>

class MprpcChannel : public google::protobuf::RpcChannel
{
public:
    // 重写channel的callmethod方法，以便初始化server_stud 然后就可以调用该方法
    void CallMethod(const google::protobuf::MethodDescriptor* method,
        google::protobuf::RpcController* controller, 
        const google::protobuf::Message* request,
        google::protobuf::Message* response, 
        google::protobuf::Closure* done);
};


