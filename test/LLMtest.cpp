#include <gtest/gtest.h>
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
    //创建会话
    std::string sessionId = sdk->createSession("deepseek-chat");
    ASSERT_TRUE(!sessionId.empty());
    //获取指定会话
    auto session = sdk->getSession(sessionId);
    ASSERT_TRUE(session);
    ASSERT_EQ(session->_sessionId, sessionId);
    //发送消息
    std::string response = sdk->sendStreamMessage(sessionId,"你好!我是伊蕾娜!",callback);
    ASSERT_FALSE(response.empty());
    response = sdk->sendStreamMessage(sessionId,"我是谁?",callback);
    ASSERT_FALSE(response.empty());
}

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
//     //获取会话列表
//     auto sessions = sdk->getAllSessionLists();
//     ASSERT_EQ(sessions.size(), 1);
//     //给第一个会话发送消息
//     std::string response = sdk->sendStreamMessage(sessions[0],"你再说说我是谁呢?",callback);
//     ASSERT_FALSE(response.empty());
// }

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    linne::Logger::initLogger("LLMtest", "log", spdlog::level::debug);
    return RUN_ALL_TESTS();
}