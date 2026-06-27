#pragma once
#include <functional>
#include"common.h"
#include"LLMManager.h"
#include"SessionManager.h"

namespace ai_chat_sdk
{
    class ChatSDK
    {
        public:
            //初始化模型
            bool initModel(const std::vector<std::shared_ptr<Config>>& configs);
            //是否初始化
            bool isInitialized() const;
            //创建会话
            std::string createSession(const std::string &modelName);
            //获取指定会话
            const std::shared_ptr<Session> getSession(const std::string &sessionId);
            //获取所有会话列表
            std::vector<std::string> getAllSessionLists();
            //删除指定会话
            bool deleteSession(const std::string &sessionId);
            //获取可用模型信息
            std::vector<std::shared_ptr<ModelInfo>> getAvailableModels();
            //发送消息,全量返回
            std::string sendMessage(const std::string &sessionId, const std::string &message);
            //发送消息,流式返回
            std::string sendStreamMessage(const std::string &sessionId, const std::string &message,
                                                std::function<void(const std::string&,bool)> callback);
        private:
            //注册所支持的模型
            void registerModels(const std::vector<std::shared_ptr<Config>>& configs);
            //初始化所支持的模型
            void initModels(const std::vector<std::shared_ptr<Config>>& configs);
            //初始化API云端模型
            void initApiModels(const std::shared_ptr<ApiConfig>& apiConfig);
            //初始化ollama本地模型
            void initOllamaModels(const std::shared_ptr<OllamaConfig>& ollamaConfig);
        private:
            bool _initialized = false;                                              // 是否初始化
            std::unordered_map<std::string,std::shared_ptr<Config>> _modelConfigs;  // 模型配置映射
            LLMManager _llmManager;                                                 // LLM管理器
            SessionManager _sessionManager;                                         // 会话管理器  
    };
}// namespace ai_chat_sdk