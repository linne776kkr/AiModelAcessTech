#pragma once
#include <string>
#include <map>
#include <vector>
#include <functional>
#include "common.h"
namespace ai_chat_sdk
{
    class LLMProvider
    {
    public:
        // 初始化模型，传入模型配置信息
        virtual void initModel(const std::map<std::string, std::string> &modelConfig) = 0;
        // 检测模型是否有效
        virtual bool isAvailable() const = 0;
        // 获取模型名称
        virtual std::string getModelName() const = 0;
        // 发送消息,全量返回
        virtual void sendMessage(const std::vector<Message> &messages, const std::map<std::string, std::string> &requestParam) = 0;
        // 发送消息，增量返回
        virtual void seneMessageStream(const std::vector<Message> &messages,
                                       const std::map<std::string, std::string> &requestParam,
                                       std::function<void(const std::string &, bool)> callback) = 0;

    private:
        bool _isAvailable = false; // 标记模型是否有效
        std::string _apiKey;       // API密钥
        std::string _endpoint;     // 模型API地址
    };
}