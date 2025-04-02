#include "include/mprpcconfig.h"
#include <iostream>

void MprpcConfig::Trim(std::string &src_buf)
{
    int idx = src_buf.find_first_not_of(' ');
    if(idx != -1)    // 找到了
    {
        // 说明前面有空格,则截掉
        src_buf = src_buf.substr(idx, src_buf.size() - idx);    
    }

    // 去除最后面的空格
    idx = src_buf.find_last_not_of(' ');

    if(idx != -1)   // 找到了
    {
        // 说明后面有空格，则截掉
        src_buf = src_buf.substr(0, idx + 1);
    }

}

void MprpcConfig::LoadConfigFile(const char * config_file)
{
    FILE *pf = fopen(config_file, "r");
    if(nullptr == pf)
    {
        std::cout << config_file << "is not exist!" << std::endl; 
        exit(EXIT_FAILURE);
    }
  
    while(!feof(pf))    // 非空文件
    {
        char buf[512] = {0};
        fgets(buf, 512, pf);

        // 去除字符串前面多余的空格
        std::string read_buf(buf);
    
        Trim(read_buf);

        // 判断#的注释
        if(read_buf[0] == '#' || read_buf.empty())
        {
            continue;
        }

        // 解析配置项
        int idx = read_buf.find('=');
        if(idx == -1)   // 说明没有等号，即配置没值
        {
            continue;
        }


        std::string key;
        std::string value;
        key = read_buf.substr(0, idx);
        // 清理一下key的空格
        Trim(key);

        
        int endkey = read_buf.find('\n');
        value = read_buf.substr(idx + 1, endkey - idx - 1);  // 末尾包括了一个\n
        // 清理一下value的空格
        Trim(value);
        std::cout << value << std::endl;
        m_configMap.insert({key, value});
    }

}

std::string MprpcConfig::Load(const std::string &key)
{
    auto it = m_configMap.find(key);
    if(it == m_configMap.end()) return " ";
    return it->second;
}