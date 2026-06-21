#pragma once
#include <string>
#include <map>
#include <vector>
#include <functional>
#include "common.h"
#include "util/myLog.h"
namespace ai_chat_sdk
{
    class LLMProvider
    {
    public:
        // 初始化模型，传入模型配置信息
        virtual bool initModel(const std::map<std::string, std::string> &modelConfig) = 0;
        // 检测模型是否有效
        virtual bool isAvailable() const = 0;
        // 获取模型名称
        virtual std::string getModelName() const = 0;
        // 获取模型描述信息
        virtual std::string getModelDesc() const = 0;
        // 发送消息,全量返回
        virtual std::string sendMessage(const std::vector<Message> &messages, const std::map<std::string, std::string> &requestParam) = 0;
        // 发送消息，增量返回
        // callback:第一个参数为增量参数,第二个参数为是否结束标志
        virtual std::string sendMessageStream(const std::vector<Message> &messages,
                                              const std::map<std::string, std::string> &requestParam,
                                              std::function<void(const std::string &, bool)> callback) = 0;

    protected:
        bool _isAvailable = false; // 标记模型是否有效
        std::string _apiKey;       // API密钥
        std::string _endpoint;     // 模型API地址
    };
}