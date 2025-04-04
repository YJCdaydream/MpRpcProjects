#pragma once
#include <google/protobuf/service.h>
#include <string>

class MprpcController   : public google::protobuf::RpcController
{
private:
    bool m_failed;  // RPC方法执行过程的状态
    std::string m_errText;  // RPC方法执行过程中的错误信息
public:
    MprpcController(/* args */);

    void Reset();

    bool Failed() const;

    std::string ErrorText() const;

    void SetFailed(const std::string& reason);

    // 不实现
    void StartCancel();
    bool IsCanceled() const ;
    void NotifyOnCancel(google::protobuf::Closure* callback);
};  



