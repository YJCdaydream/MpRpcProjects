#pragma once
#include <unordered_map>
#include <string>

// 框架读取配置文件类

class MprpcConfig
{
private:
    std::unordered_map<std::string, std::string> m_configMap;

    // 取掉前后空格
    void Trim(std::string &src_buf);
public:
    // 加载配置文件
    void LoadConfigFile(const char * config_file);
    
    // 从m_configMap找到对应的配置参数
    std::string Load(const std::string &key);
};

