基于c++实现的轻量的RPC框架

使用到的技术
    1.muduo网络库   => 作为服务端的网络连接
    2.zookeeper    => 在zookeeper上注册服务端发布的网络服务
    3.protobuf     => 使用作为反序列化和序列化的工具


整体项目框架
    autobuild.sh => shell自动编译脚本
    bin          => 编译完将在bin目录下生成可执行文件，以及包含项目的配置文件test.conf，默认配置将会运行在本地
    lib          => 编译完将在此生成动态库，以及会将.h文件放入其中
    build        => cmake生成的编译文件，可删除并在此执行cmake编译
    example      => 用于测试的服务端代码callee 和 用户端代码caller
    src          => 整个项目的源码


项目执行流程
    rpc服务调用方（user）                   rpc服务提供方（server）                         ZooKeeper
                                                |  <-----------通过session连接----------->  |
                                        1. RpcProvider::NotifyService 初始化
                                        将通过维护一个服务名 与服务类 以及方法名和方法的MAP表

                                        2. RpcProvider::run 注册节点服务
                                        然后将MAP表中的方法全部注册到znode节点上 ------->  节点格式类似于 服务名（永久节点）-->方法名（临时节点）

                                        3. RpcProvider::run 通过muduo开启服务端服务
                                        创建了四个线程的线程池，一个I/O和三个Work线程
                                        start和loop，开启RPC服务节点启动


    4. 调用具体服务类的服务方法（例：login方法
    都将会调用UserServiceRpc_Stub中的RpcChannel的回调函数，在其中再获取
    具体的服务对象和服务方法

    5. 查询znode 获取是否存在该服务  ------------------------------------------------------->    |
    返回服务所部署的ip和port      <------------------------------------------------------     |

    6. 创建规定格式的请求头（headsize + rpcheader + args
    通过protobuf序列化后
    根据ip和port发送rpc调用请求（普通socket通信）--> |

                                        7.接收请求并返回响应 
                                        （1）muduo的OnMessage回调获取到网络字符流
                                        反序列化，获取到请求信息，及具体的服务类和服务方法
                                        （2）调用具体的方法，生成Response，并序列化
                                        （3）通过closure 绑定的回调函数 RpcProvider::SendRpcResponse
                                            发送回给调用方 


    7. 先通过MprpcController检测是否调用成功
    成功则打印结果，否则失败程序错误退出


项目额外功能
    集成了一个日志系统
    ：通过队列和互斥锁实现
        通过std::lock_guard来管理锁，写队列时上锁，写完自动放锁
        出队上锁，队为空 释放锁 否则写出
      创建一个线程来写入文件



项目编译和运行
    ./autobuild.sh 
    cd bin 
    ./provider -i test.conf
    ./consumer -i test.conf
    或 
    cd build 
    cmake ..
    make
    cd bin 
    ./provider -i test.conf
    ./consumer -i test.conf






