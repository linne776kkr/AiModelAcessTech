#include"../include/LLMManager.h"
#include <sys/ucontext.h>
namespace ai_chat_sdk
{
    //注册LLM提供者
    bool LLMManager::registerProvider(const std::string &modelName,std::unique_ptr<LLMProvider> provider)
    {
        //检查provider是否为空
        if(provider.get() == nullptr)
        {
            ERR("LLMProvider is nullptr");
            return false;
        }
        _providers[modelName] = std::move(provider);
        //添加模型信息
        _modelInfos[modelName] = ModelInfo(modelName);
        INFO("Register LLMProvider for model: {}", modelName);
        return true;
    }
    //初始化指定模型
    bool LLMManager::initModel(const std::string &modelName,const std::map<std::string,std::string> &modelConfig)
    {
        auto it = _providers.find(modelName);
        if(it == _providers.end())
        {
           WARN("LLMProvider not registered for model: {}", modelName);
            return false;
        }
        //初始化模型
        bool isInit = it->second->initModel(modelConfig);
        if(isInit)
        {
            //更新模型信息
            _modelInfos[modelName]._isAvailable = isInit;
            _modelInfos[modelName]._modelDesc = it->second->getModelDesc();
            return true;
        }
        return false;
    }
    //获取可用模型
    std::vector<ModelInfo> LLMManager::getAvailableModels()
    {
        std::vector<ModelInfo> models;
        for(auto &it : _modelInfos)
        {
            if(!it.second._isAvailable)
            {
                models.push_back(it.second);
            }
        }
        return models;
    }
    //检查模型是否可用
    bool LLMManager::isModelAvailable(const std::string &modelName)
    {
        auto it = _providers.find(modelName);
        if(it == _providers.end())
        {
            WARN("LLMProvider not registered for model: {}", modelName);
            return false;
        }
        return it->second->isAvailable();
    }
    //发送消息给指定模型
    std::string LLMManager::sendMessage(const std::string &modelName,
                                        const std::vector<Message>& messages,
                                        const std::map<std::string,std::string>& requestParam)
    {
        auto it = _providers.find(modelName);
        if(it == _providers.end())
        {
            WARN("LLMProvider not registered for model: {}", modelName);
            return "";
        }
        if(!it->second->isAvailable())
        {
            ERR("Model is not available for model: {}", modelName);
            return "";
        }
        return it->second->sendMessage(messages,requestParam);
    }
    //发送流式返回消息给指定模型
    std::string LLMManager::sendStreamMessage( const std::string& modelName,
                            const std::vector<Message> &messages,
                            const std::map<std::string, std::string> &requestParam,
                            std::function<void(const std::string &, bool)> callback)
    {
        auto it = _providers.find(modelName);
        if(it == _providers.end())
        {
            WARN("LLMProvider not registered for model: {}", modelName);
            return "";
        }
        if(!it->second->isAvailable())
        {
            ERR("Model is not available for model: {}", modelName);
            return "";
        }
        return it->second->sendMessageStream(messages,requestParam,callback);
    }
}// namespace ai_chat_sdk
