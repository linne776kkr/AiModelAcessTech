#pragma once

#include <ctime>
#include <string>
#include <vector>

namespace ai_chat_sdk
{
    // 消息结构
    struct Message
    {
        std::string _messageId; // 消息id
        std::string _role;      // 消息角色
        std::string _content;   // 消息内容
        std::string _timestamp; // 消息时间戳

        // 构造函数
        Message(const std::string &role, const std::string &content)
            : _role(role), _content(content) {}
    };
    // 模型的公共配置信息
    struct Config
    {
        std::string _modelName; // 模型名称
        double _temperature;    // 温度参数
        int _maxTokens;         // 最大token数
    };
    // 通过api方式接入云端模型
    struct ApiConfig : public Config
    {
        std::string _apiKey; // api密钥
    };
    // 通过ollama方式接入本地模型--不需要配置apikey
    // LLM信息
    struct ModelInfo
    {
        std::string _modelName;    // 模型名称
        std::string _modelDesc;    // 模型描述
        std::string _provider;     // 模型提供方
        std::string _endpoint;     // 模型endpoint地址
        bool _isAvailable = false; // 模型是否可用

        ModelInfo(const std::string &modelName, const std::string &modelDesc,
                  const std::string &provider, const std::string &endpoint)
            : _modelName(modelName), _modelDesc(modelDesc), _provider(provider),
              _endpoint(endpoint) {}
    };
    // 会话信息
    struct Session
    {
        std::string _sessionId;         // 会话id
        std::string _modelName;         // 模型名称
        std::vector<Message> _messages; // 会话消息列表
        std::time_t _createTime;        // 会话创建时间
        std::time_t _lastUpdateTime;    // 会话最后更新时间
    };
} // namespace ai_chat_sdk