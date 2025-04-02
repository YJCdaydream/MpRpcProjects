#include "mprpcapplication.h"
#include "string"
#include "cstdlib"
#include "unistd.h"
#include<iostream>

MprpcConfig MprpcApplication::m_config;

void ShowArgsHelp()
{
    std::cout<< "format: command -i  <configfile>" << std::endl;
}

MprpcApplication::MprpcApplication(){};

void MprpcApplication::Init(int argc, char ** argv)
{
    if(argc < 2)    // 说明没参数，直接退出
    {
        ShowArgsHelp(); // 提示一下命令
        exit(EXIT_FAILURE); // 终止程序运行
    }

    int c = 0;
    std::string config_file;
    while ((c = getopt(argc, argv, "i:")) != -1)
    {
        switch (c)
        {
        case 'i':   // 表示输入了i 且i后面带参数，且参数将赋给optarg，optarg再赋给config_file
            config_file = optarg;
            break;
        case '?':   // 表示未出现i，则直接退出
            ShowArgsHelp();
            exit(EXIT_FAILURE);
        case ':':   // 说明未带参数，不服规则则直接退出
            ShowArgsHelp();
            exit(EXIT_FAILURE);
        default:
            break;
        }
    }
    
    // 开始加载配置文件
    m_config.LoadConfigFile(config_file.c_str());

    // std::cout << "rpcserverip:"<<m_config.Load("rpcserverip") <<std::endl;
    // std::cout << "rpcserverport:"<<m_config.Load("rpcserverport") <<std::endl;
    // std::cout << "zookeeperip:"<< m_config.Load("zookeeperip") <<std::endl;
    // std::cout << "zookeeperport:"<<m_config.Load("zookeeperport") <<std::endl;

}


MprpcApplication & MprpcApplication::GetInstance(){
    static MprpcApplication app;
    return app;
}

MprpcConfig & MprpcApplication::GetMprpcconfig()
{
    return m_config;
}