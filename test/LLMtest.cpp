#include "../sdk/include/DeepSeekProvider.h"
#include <gtest/gtest.h>
#include "../sdk/include/util/myLog.h"

TEST(DeepSeekProviderTest, sendMessage)
{
    // 构造deepseek对象
    auto provider = std::make_shared<ai_chat_sdk::DeepSeekProvider>();
    ASSERT_NE(provider, nullptr);
    // 构造模型配置
    std::map<std::string, std::string> modelParam;
    modelParam["apiKey"] = std::getenv("DEEP_SEEK_API_KEY");
    modelParam["endpoint"] = "https://api.deepseek.com";
    provider->initModel(modelParam);
    ASSERT_EQ(provider->isAvailable(), true);
    // 构造消息
    std::string response = provider->sendMessage({{"user", "你好"}}, {{"temperature", "0.5"}, {"maxTokens", "2048"}});
    ASSERT_FALSE(response.empty());
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    linne::Logger::initLogger("LLMtest", "stdout", spdlog::level::debug);
    return RUN_ALL_TESTS();
}