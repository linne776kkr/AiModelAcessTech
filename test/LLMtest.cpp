#include <gtest/gtest.h>
#include "../sdk/include/util/myLog.h"
#include "../sdk/include/ChatSDK.h"

void callback(const std::string &content, bool isFinish)
{
    if (isFinish)
    {
        std::cout<<std::endl;        
    }
    else
    {
        std::cout << content << std::flush;
    }
}

// TEST(DeepSeekProviderTest, sendMessage)
// {
//     // 构造deepseek对象
//     auto provider = std::make_shared<ai_chat_sdk::DeepSeekProvider>();
//     ASSERT_NE(provider, nullptr);
//     // 构造模型配置
//     std::map<std::string, std::string> modelParam;
//     modelParam["apiKey"] = std::getenv("DEEP_SEEK_API_KEY");
//     modelParam["endpoint"] = "https://api.deepseek.com";
//     provider->initModel(modelParam);
//     ASSERT_EQ(provider->isAvailable(), true);
//     // 构造消息
//     // std::string response = provider->sendMessage({{"user", "你好"}}, {{"temperature", "0.5"}, {"maxTokens", "2048"}});
//     std::string response = provider->sendMessageStream({{"user", "你好,帮我生成一篇700字小作文"}}, {{"temperature", "0.5"}, {"maxTokens", "2048"}}, callback);
//     ASSERT_FALSE(response.empty());
// }

// TEST(ChatGPTProviderTest, sendMessage)
// {
//     // 构造ChatGPT对象
//     auto provider = std::make_shared<ai_chat_sdk::ChatGPTProvider>();
//     ASSERT_NE(provider, nullptr);
//     // 构造模型配置
//     std::map<std::string, std::string> modelParam;
//     modelParam["apiKey"] = std::getenv("OPENAI_API_KEY");
//     modelParam["endpoint"] = "https://api.openai.com";
//     provider->initModel(modelParam);
//     ASSERT_EQ(provider->isAvailable(), true);
//     // 构造消息
//     //std::string response = provider->sendMessage({{"user", "你好"}}, {{"temperature", "0.5"}, {"maxTokens", "2048"}});
//     std::string response = provider->sendMessageStream({{"user", "你好,帮我生成一篇700字小作文,有关魔女之旅伊蕾娜的"}}, {{"temperature", "0.5"}, {"maxTokens", "2048"}}, callback);
//     ASSERT_FALSE(response.empty());
// }

// TEST(OllamaProviderTest, sendMessage)
// {
//     // 构造Ollama对象
//     auto provider = std::make_shared<ai_chat_sdk::OllamaProvider>();
//     ASSERT_NE(provider, nullptr);
//     // 构造模型配置
//     std::map<std::string, std::string> modelParam;
//     modelParam["modelName"] = "deepseek-r1:1.5b";
//     modelParam["modelDesc"] = "deepseek-r1:1.5b是一款轻量级的聊天模型，支持中文和英文,适合简单的对话和任务执行";
//     modelParam["endpoint"] = "http://127.0.0.1:11434";
//     provider->initModel(modelParam);
//     ASSERT_EQ(provider->isAvailable(), true);
//     // 构造消息
//     //std::string response = provider->sendMessage({{"user", "你好"}}, {{"temperature", "0.5"}, {"maxTokens", "2048"}});
//     std::string response = provider->sendMessageStream({{"user", "你好,帮我生成一篇700字小作文,有关魔女之旅伊蕾娜的"}}, {{"temperature", "0.5"}, {"maxTokens", "2048"}}, callback);
//     ASSERT_FALSE(response.empty());
// }

// TEST(ChatSDKTest, sendMessage)
// {
//     auto sdk = std::make_shared<ai_chat_sdk::ChatSDK>();
//     ASSERT_TRUE(sdk);
//     //配置支持的模型参数,deepseek-chat,gpt-4o-mini,ollama本地接入的deepseek-r1:1.5b
//     std::vector<std::shared_ptr<ai_chat_sdk::Config>> configs;
//     std::shared_ptr<ai_chat_sdk::ApiConfig> deepseekConfig = std::make_shared<ai_chat_sdk::ApiConfig>();
//     deepseekConfig->_modelName = "deepseek-chat";
//     deepseekConfig->_temperature = 0.7;
//     deepseekConfig->_maxTokens = 2048;
//     deepseekConfig->_apiKey = std::getenv("DEEP_SEEK_API_KEY");
//     configs.push_back(deepseekConfig);
//     std::shared_ptr<ai_chat_sdk::ApiConfig> gpt4oConfig = std::make_shared<ai_chat_sdk::ApiConfig>();
//     gpt4oConfig->_modelName = "gpt-4o-mini";
//     gpt4oConfig->_temperature = 0.7;
//     gpt4oConfig->_maxTokens = 2048;
//     gpt4oConfig->_apiKey = std::getenv("OPENAI_API_KEY");
//     configs.push_back(gpt4oConfig);
//     std::shared_ptr<ai_chat_sdk::OllamaConfig> ollamaConfig = std::make_shared<ai_chat_sdk::OllamaConfig>();
//     ollamaConfig->_modelName = "deepseek-r1:1.5b";
//     ollamaConfig->_temperature = 0.7;
//     ollamaConfig->_maxTokens = 2048;
//     ollamaConfig->_modelDesc = "deepseek-r1:1.5b是一款轻量级的聊天模型，支持中文和英文,适合简单的对话和任务执行";
//     ollamaConfig->_endpoint = "http://127.0.0.1:11434";
//     configs.push_back(ollamaConfig);
//     sdk->initModel(configs);
//     ASSERT_TRUE(sdk->isInitialized());
//     //创建会话
//     std::string sessionId = sdk->createSession("deepseek-chat");
//     ASSERT_TRUE(!sessionId.empty());
//     //获取指定会话
//     auto session = sdk->getSession(sessionId);
//     ASSERT_TRUE(session);
//     ASSERT_EQ(session->_sessionId, sessionId);
//     //发送消息
//     std::string response = sdk->sendStreamMessage(sessionId,"你好!我是伊蕾娜!",callback);
//     ASSERT_FALSE(response.empty());
//     response = sdk->sendStreamMessage(sessionId,"我是谁?",callback);
//     ASSERT_FALSE(response.empty());
// }

TEST(ChatSDKTest, sendMessage)
{
    auto sdk = std::make_shared<ai_chat_sdk::ChatSDK>();
    ASSERT_TRUE(sdk);
    //配置支持的模型参数,deepseek-chat,gpt-4o-mini,ollama本地接入的deepseek-r1:1.5b
    std::vector<std::shared_ptr<ai_chat_sdk::Config>> configs;
    std::shared_ptr<ai_chat_sdk::ApiConfig> deepseekConfig = std::make_shared<ai_chat_sdk::ApiConfig>();
    deepseekConfig->_modelName = "deepseek-chat";
    deepseekConfig->_temperature = 0.7;
    deepseekConfig->_maxTokens = 2048;
    deepseekConfig->_apiKey = std::getenv("DEEP_SEEK_API_KEY");
    configs.push_back(deepseekConfig);
    std::shared_ptr<ai_chat_sdk::ApiConfig> gpt4oConfig = std::make_shared<ai_chat_sdk::ApiConfig>();
    gpt4oConfig->_modelName = "gpt-4o-mini";
    gpt4oConfig->_temperature = 0.7;
    gpt4oConfig->_maxTokens = 2048;
    gpt4oConfig->_apiKey = std::getenv("OPENAI_API_KEY");
    configs.push_back(gpt4oConfig);
    std::shared_ptr<ai_chat_sdk::OllamaConfig> ollamaConfig = std::make_shared<ai_chat_sdk::OllamaConfig>();
    ollamaConfig->_modelName = "deepseek-r1:1.5b";
    ollamaConfig->_temperature = 0.7;
    ollamaConfig->_maxTokens = 2048;
    ollamaConfig->_modelDesc = "deepseek-r1:1.5b是一款轻量级的聊天模型，支持中文和英文,适合简单的对话和任务执行";
    ollamaConfig->_endpoint = "http://127.0.0.1:11434";
    configs.push_back(ollamaConfig);
    sdk->initModel(configs);
    ASSERT_TRUE(sdk->isInitialized());
    //获取会话列表
    auto sessions = sdk->getAllSessionLists();
    ASSERT_EQ(sessions.size(), 1);
    //给第一个会话发送消息
    std::string response = sdk->sendStreamMessage(sessions[0],"你再说说我是谁呢?",callback);
    ASSERT_FALSE(response.empty());
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    linne::Logger::initLogger("LLMtest", "log", spdlog::level::debug);
    return RUN_ALL_TESTS();
}