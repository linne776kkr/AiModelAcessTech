#include"../include/ChatSDK.h"
#include"../include/DeepSeekProvider.h"
#include"../include/ChatGPTProvider.h"
#include"../include/OllamaProvider.h"
namespace ai_chat_sdk
{
    //初始化模型
    bool ChatSDK::initModel(const std::vector<std::shared_ptr<Config>>& configs)
    {
        //注册模型
        registerModels(configs);
        //初始化模型
        initModels(configs);
        _initialized = true;
        INFO("ChatSDK::initModel: model initialized");
        return true;
    }
    //是否初始化
    bool ChatSDK::isInitialized() const
    {
        return _initialized;
    }
    //创建会话
    std::string ChatSDK::createSession(const std::string &modelName)
    {
        if(!_initialized)
        {
            ERR("ChatSDK::createSession: model not initialized");
            return "";
        }
        if(!_llmManager.isModelAvailable(modelName))
        {
            ERR("ChatSDK::createSession: model not available");
            return "";
        }
        return _sessionManager.createSession(modelName);
    }
    //获取指定会话
    const std::shared_ptr<Session> ChatSDK::getSession(const std::string &sessionId)
    {
        if(!_initialized)
        {
            ERR("ChatSDK::getSession: model not initialized");
            return nullptr;
        }
        return _sessionManager.getSession(sessionId);
    }
    //获取所有会话列表
    std::vector<std::string> ChatSDK::getAllSessionLists()
    {
        if(!_initialized)
        {
            ERR("ChatSDK::getAllSessionLists: model not initialized");
            return {};
        }
        return _sessionManager.getSessionList();
    }
    //删除指定会话
    bool ChatSDK::deleteSession(const std::string &sessionId)
    {
        if(!_initialized)
        {
            ERR("ChatSDK::deleteSession: model not initialized");
            return false;
        }
        return _sessionManager.deleteSession(sessionId);
    }
    //获取可用模型信息
    std::vector<std::shared_ptr<ModelInfo>> ChatSDK::getAvailableModels()
    {
        if(!_initialized)
        {
            ERR("ChatSDK::getAvailableModels: model not initialized");
            return {};
        }
        return _llmManager.getAvailableModels();
    }
    //发送消息,全量返回
    std::string ChatSDK::sendMessage(const std::string &sessionId, const std::string &message)
    {
        if(!_initialized)
        {
            ERR("ChatSDK::sendMessage: model not initialized");
            return "";
        }
        //获取会话信息
        auto session = _sessionManager.getSession(sessionId);
        if(!session)
        {
            ERR("ChatSDK::sendMessage: session not found");
            return "";
        }
        //向会话新增用户消息
        Message userMessage;
        userMessage._role = "user";
        userMessage._content = message;
        bool ret = _sessionManager.addMessage(sessionId,userMessage);
        if(!ret)
        {
            ERR("ChatSDK::sendMessage: add user message failed");
            return "";
        }
        //构建会话历史消息
        std::vector<std::shared_ptr<Message>> historyMessages;
        historyMessages = _sessionManager.getHistoryMessage(session);
        //构建请求参数
        auto it = _modelConfigs.find(session->_modelName);
        if(it==_modelConfigs.end())
        {
            ERR("ChatSDK::sendMessage: model not found");
            return "";
        }
        std::map<std::string, std::string> requestParam;
        if(it->second->_temperature!=0.0) requestParam["temperature"] = std::to_string(it->second->_temperature);
        if(it->second->_maxTokens!=0) requestParam["max_tokens"] = std::to_string(it->second->_maxTokens);
        //向模型发送请求
        std::string response = _llmManager.sendMessage(session->_modelName,historyMessages,requestParam);
        if(response.empty())
        {
            ERR("ChatSDK::sendMessage: model response is empty");
            return "";
        }
        //向会话新增回应消息
        Message responseMessage;
        responseMessage._role = "assistant";
        responseMessage._content = response;
        _sessionManager.addMessage(sessionId,responseMessage);
        INFO("ChatSDK::sendMessage: model response: %s",response.c_str());
        return response;
    }
    //发送消息,流式返回
    std::string ChatSDK::sendStreamMessage(const std::string &sessionId, const std::string &message,
                                                std::function<void(const std::string&,bool)> callback)
    {
        if(!_initialized)
        {
            ERR("ChatSDK::sendStreamMessage: model not initialized");
            return "";
        }
        //获取会话信息
        auto session = _sessionManager.getSession(sessionId);
        if(!session)
        {
            ERR("ChatSDK::sendStreamMessage: session not found");
            return "";
        }
        //向会话新增用户消息
        Message userMessage;
        userMessage._role = "user";
        userMessage._content = message;
        bool ret = _sessionManager.addMessage(sessionId,userMessage);
        if(!ret)
        {
            ERR("ChatSDK::sendStreamMessage: add user message failed");
            return "";
        }
        //构建会话历史消息
        std::vector<std::shared_ptr<Message>> historyMessages;
        historyMessages = _sessionManager.getHistoryMessage(session);
        //构建请求参数
        auto it = _modelConfigs.find(session->_modelName);
        if(it==_modelConfigs.end())
        {
            ERR("ChatSDK::sendStreamMessage: model not found");
            return "";
        }
        std::map<std::string, std::string> requestParam;
        if(it->second->_temperature!=0.0) requestParam["temperature"] = std::to_string(it->second->_temperature);
        if(it->second->_maxTokens!=0) requestParam["max_tokens"] = std::to_string(it->second->_maxTokens);
        //向模型发送请求
        std::string response = _llmManager.sendStreamMessage(session->_modelName,historyMessages,requestParam,callback);
        if(response.empty())
        {
            ERR("ChatSDK::sendStreamMessage: model response is empty");
            return "";
        }
        //向会话新增回应消息
        Message responseMessage;
        responseMessage._role = "assistant";
        responseMessage._content = response;
        _sessionManager.addMessage(sessionId,responseMessage);
        INFO("ChatSDK::sendStreamMessage: model response: {}",response);
        return response;                              
    }
    //注册所支持的模型
    void ChatSDK::registerModels(const std::vector<std::shared_ptr<Config>>& configs)
    {
        //注册deepseek-chat模型
        if(!_llmManager.isModelAvailable("deepseek-chat"))
        {
            std::unique_ptr<DeepSeekProvider> provider = std::make_unique<DeepSeekProvider>();
            _llmManager.registerProvider("deepseek-chat",std::move(provider));
        }
        //注册gpt-4o-mini模型
        if(!_llmManager.isModelAvailable("gpt-4o-mini"))
        {
            std::unique_ptr<ChatGPTProvider> provider = std::make_unique<ChatGPTProvider>();
            _llmManager.registerProvider("gpt-4o-mini",std::move(provider));
        }
        //注册ollama本地模型
        for(const auto &config : configs)
        {
            auto ollamaConfig = std::dynamic_pointer_cast<OllamaConfig>(config);
            if(ollamaConfig)
            {
                if(!_llmManager.isModelAvailable(ollamaConfig->_modelName))
                {
                    std::unique_ptr<OllamaProvider> provider = std::make_unique<OllamaProvider>();
                    _llmManager.registerProvider(ollamaConfig->_modelName,std::move(provider));
                }
            }
        }
    }
    //初始化所支持的模型
    void ChatSDK::initModels(const std::vector<std::shared_ptr<Config>>& configs)
    {
        for(const auto &config : configs)
        {
            if(auto apiConfig = std::dynamic_pointer_cast<ApiConfig>(config))
            {
                //初始化API云端模型
                initApiModels(apiConfig);
            }
            else if(auto ollamaConfig = std::dynamic_pointer_cast<OllamaConfig>(config))
            {
                //初始化ollama本地模型
                initOllamaModels(ollamaConfig);
            }
            else
            {
                //未知模型类型
                WARN("Unknown model type: {}",config->_modelName);
            }
        }
    }
    //初始化API云端模型
    void ChatSDK::initApiModels(const std::shared_ptr<ApiConfig>& apiConfig)
    {
        //检查参数是否存在
        if(apiConfig.get()==nullptr)
        {
            WARN("API key is empty for API model");
            return;
        }
        if(apiConfig->_apiKey.empty()||apiConfig->_modelName.empty())
        {
            WARN("API key or model name is empty for API model");
            return;
        }
        std::map<std::string, std::string> modelConfig;
        modelConfig["apiKey"] = apiConfig->_apiKey;
        //初始化API云端模型
        _llmManager.initModel(apiConfig->_modelName,modelConfig);
        //更新配置信息
        _modelConfigs[apiConfig->_modelName] = apiConfig;
    }
    //初始化ollama本地模型
    void ChatSDK::initOllamaModels(const std::shared_ptr<OllamaConfig>& ollamaConfig)
    {
        //检查参数是否存在
        if(ollamaConfig.get()==nullptr)
        {
            WARN("Model name is empty for Ollama model");
            return;
        }
        if(ollamaConfig->_modelName.empty()||ollamaConfig->_endpoint.empty())
        {
            WARN("Model name or endpoint is empty for Ollama local model");
            return;
        }
        std::map<std::string, std::string> modelConfig;
        modelConfig["modelName"] = ollamaConfig->_modelName;
        modelConfig["modelDesc"] = ollamaConfig->_modelDesc;
        modelConfig["endpoint"] = ollamaConfig->_endpoint;
        //初始化ollama本地模型
        _llmManager.initModel(ollamaConfig->_modelName,modelConfig);
        //更新配置信息
        _modelConfigs[ollamaConfig->_modelName] = ollamaConfig;
    }
}// namespace ai_chat_sdk