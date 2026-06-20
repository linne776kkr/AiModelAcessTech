#include "../sdk/include/DeepSeekProvider.h"
#include "../sdk/include/ChatGPTProvider.h"
#include <gtest/gtest.h>
#include "../sdk/include/util/myLog.h"

void callback(const std::string &content, bool isFinish)
{
    if (isFinish)
    {
        INFO("Stream response finished");
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
//     std::string response = provider->seneMessageStream({{"user", "你好,帮我生成一篇700字小作文"}}, {{"temperature", "0.5"}, {"maxTokens", "2048"}}, callback);
//     ASSERT_FALSE(response.empty());
// }

TEST(ChatGPTProviderTest, sendMessage)
{
    // 构造ChatGPT对象
    auto provider = std::make_shared<ai_chat_sdk::ChatGPTProvider>();
    ASSERT_NE(provider, nullptr);
    // 构造模型配置
    std::map<std::string, std::string> modelParam;
    modelParam["apiKey"] = std::getenv("OPENAI_API_KEY");
    modelParam["endpoint"] = "https://api.openai.com";
    provider->initModel(modelParam);
    ASSERT_EQ(provider->isAvailable(), true);
    // 构造消息
    //std::string response = provider->sendMessage({{"user", "你好"}}, {{"temperature", "0.5"}, {"maxTokens", "2048"}});
    std::string response = provider->seneMessageStream({{"user", "你好,帮我生成一篇700字小作文,有关魔女之旅伊蕾娜的"}}, {{"temperature", "0.5"}, {"maxTokens", "2048"}}, callback);
    ASSERT_FALSE(response.empty());
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    linne::Logger::initLogger("LLMtest", "log", spdlog::level::debug);
    return RUN_ALL_TESTS();
}