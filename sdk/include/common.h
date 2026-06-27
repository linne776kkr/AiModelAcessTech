#pragma once

#include <ctime>
#include <string>
#include <vector>
#include <memory>

namespace ai_chat_sdk
{
    // 消息结构
    struct Message
    {
        std::string _messageId; // 消息id
        std::string _role;      // 消息角色
        std::string _content;   // 消息内容
        std::time_t _timestamp; // 消息时间戳
    };
    // 模型的公共配置信息
    struct Config
    {
        std::string _modelName; // 模型名称
        double _temperature;    // 温度参数
        int _maxTokens;         // 最大token数

        virtual ~Config() = default;//用于向下转换时的安全性
        Config(const std::string &modelName, double temperature, int maxTokens)
            : _modelName(modelName), _temperature(temperature), _maxTokens(maxTokens) {}
        Config(){}
    };
    // 通过api方式接入云端模型
    struct ApiConfig : public Config
    {
        std::string _apiKey;    // api密钥
        //构造函数
        ApiConfig(const std::string &modelName, double temperature, int maxTokens,
                  const std::string &apiKey)
            : Config(modelName, temperature, maxTokens), _apiKey(apiKey) {}
        ApiConfig(){}
    };
    // 通过ollama方式接入本地模型--不需要配置apikey
    struct OllamaConfig : public Config
    {
        std::string _modelDesc; // 模型描述
        std::string _endpoint;  // 模型endpoint地址
        //构造函数
        OllamaConfig(const std::string &modelName, double temperature, int maxTokens,
                      const std::string &modelDesc, const std::string &endpoint)
            : Config(modelName, temperature, maxTokens), _modelDesc(modelDesc),_endpoint(endpoint) {}
        OllamaConfig(){}
    };
    // LLM信息
    struct ModelInfo
    {
        std::string _modelName;    // 模型名称
        std::string _modelDesc;    // 模型描述
        std::string _provider;     // 模型提供方
        std::string _endpoint;     // 模型endpoint地址
        bool _isAvailable = false; // 模型是否可用
    };
    // 会话信息
    struct Session
    {
        std::string _sessionId;                          // 会话id
        std::string _sessionName;                        // 会话名称
        std::string _modelName;                          // 模型名称
        std::vector<std::shared_ptr<Message>> _messages; // 会话消息列表
        std::time_t _createTime;                         // 会话创建时间
        std::time_t _lastUpdateTime;                     // 会话最后更新时间
    };
} // namespace ai_chat_sdk