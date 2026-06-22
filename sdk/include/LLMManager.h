#pragma once
#include<map>
#include<memory>
#include<string>
#include"LLMProvider.h"

namespace ai_chat_sdk
{
    // LLMManager 类，用于管理多个 LLM 模型
    class LLMManager
    {
    public:
        //注册LLM提供者
        bool registerProvider(const std::string &modelName,std::unique_ptr<LLMProvider> provider);
        //初始化指定模型
        bool initModel(const std::string &modelName,const std::map<std::string,std::string> &modelConfig);
        //获取可用模型
        std::vector<ModelInfo> getAvailableModels();
        //检查模型是否可用
        bool isModelAvailable(const std::string &modelName);
        //发送消息给指定模型
        std::string sendMessage(const std::string &modelName,
                                const std::vector<Message>& messages,
                                const std::map<std::string,std::string>& requestParam);
        //发送流式返回消息给指定模型
        std::string sendStreamMessage( const std::string& modelName,
                                const std::vector<Message> &messages,
                                const std::map<std::string, std::string> &requestParam,
                                std::function<void(const std::string &, bool)> callback);
    private:
        std::map<std::string,std::unique_ptr<LLMProvider>> _providers;// 模型提供者映射表
        std::map<std::string,ModelInfo> _modelInfos;// 模型信息映射表
    };
}