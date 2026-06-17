#include <jsoncpp/json/json.h>
#include <httplib.h>

#include "../include/DeepSeekProvider.h"
#include "../include/util/myLog.h"

namespace ai_chat_sdk
{
    // 初始化模型，传入模型配置信息
    bool DeepSeekProvider::initModel(const std::map<std::string, std::string> &modelConfig)
    {
        auto it = modelConfig.find("apiKey");
        if (it == modelConfig.end())
        {
            ERR("DeepSeekProvider initModel failed, missing apiKey in modelConfig");
            _isAvailable = false;
            return false;
        }
        _apiKey = it->second;
        it = modelConfig.find("endpoint");
        if (it == modelConfig.end())
        {
            ERR("DeepSeekProvider initModel failed, missing endpoint in modelConfig");
            _isAvailable = false;
            return false;
        }
        _endpoint = it->second;
        _isAvailable = true;
        //打印apikey和endpoint
        INFO("DeepSeekProvider initModel success, apiKey:{}, endpoint:{}", _apiKey, _endpoint);
        return true;
    }
    // 检测模型是否有效
    bool DeepSeekProvider::isAvailable() const
    {
        return _isAvailable;
    }
    // 获取模型名称
    std::string DeepSeekProvider::getModelName() const
    {
        return "deepseek-chat";
    }
    // 获取模型描述信息
    std::string DeepSeekProvider::getModelDesc() const
    {
        return "一款实用性强,性能优越的中文对话模型，适用于各种对话场景，如客服、问答、闲聊等。";
    }
    // 发送消息,全量返回
    std::string DeepSeekProvider::sendMessage(const std::vector<Message> &messages, const std::map<std::string, std::string> &requestParam)
    {
        // 1.检查模型是否可用
        if (_isAvailable == false)
        {
            ERR("DeepSeekProvider sendMessage failed, model is not available");
            return "";
        }
        // 2.构造请求参数
        double temperature = 0.7;
        int maxTokens = 2048;
        auto it = requestParam.find("temperature");
        if (it != requestParam.end())
        {
            temperature = std::stod(it->second);
        }
        it = requestParam.find("maxTokens");
        if (it != requestParam.end())
        {
            maxTokens = std::stoi(it->second);
        }
        // 构造历史消息
        Json::Value messageArray(Json::arrayValue);
        for (const auto &message : messages)
        {
            Json::Value messageJson;
            messageJson["role"] = message._role;
            messageJson["content"] = message._content;
            messageArray.append(messageJson);
        }
        // 3.构造请求体
        Json::Value requestBody;
        requestBody["model"] = getModelName();
        requestBody["messages"] = messageArray;
        requestBody["temperature"] = temperature;
        requestBody["max_tokens"] = maxTokens;
        // 4.进行序列化
        Json::StreamWriterBuilder writerBuilder;
        writerBuilder["indentation"] = ""; // 去除缩进和换行
        std::string requestBodyStr = Json::writeString(writerBuilder, requestBody);
        INFO("DeepSeekProvider sendMessage request body:{}", requestBodyStr);
        // 5.使用cpp-httplib构建HTTP客户端
        httplib::Client client(_endpoint.c_str());
        client.set_connection_timeout(30, 0); // 设置连接超时时间为30秒
        client.set_read_timeout(60, 0);       // 设置读取超时时间为30秒
        // 设置请求头
        httplib::Headers headers = {
            {"Authorization", "Bearer " + _apiKey},
            {"Content-Type", "application/json"}};
        // 6.发送http请求
        auto request = client.Post("/v1/chat/completions", headers, requestBodyStr, "application/json");
        if (!request)
        {
            ERR("DeepSeekProvider sendMessage failed, http request failed");
            return "";
        }
        if (request->status != 200)
        {
            ERR("DeepSeekProvider sendMessage failed, http response status code:{}", request->status);
            return "";
        }
        INFO("DeepSeekProvider sendMessage success, response body:{}", request->body);
        INFO("DeepSeekProvider sendMessage success, response status code:{}", request->status);
        // 7.解析响应体
        Json::CharReaderBuilder readerBuilder;
        std::istringstream responseStream(request->body);
        Json::Value responseBody;
        std::string responseError;
        //将解析后的结果存到responseBody中，如果解析失败，responseError中会有错误信息
        if (Json::parseFromStream(readerBuilder, responseStream, &responseBody, &responseError))
        {
            //判断响应体中是否有choices字段，并且choices字段是一个非空数组
            if (responseBody.isMember("choices") && responseBody["choices"].isArray() && !responseBody["choices"].empty())
            {
                //从choices数组中获取第一个元素，并判断该元素是否有message字段，并且message字段中是否有content字段
                auto choice = responseBody["choices"][0];
                if (choice.isMember("message") && choice["message"].isMember("content"))
                {
                    //获取content字段的值，并将其作为函数的返回值
                    std::string content = choice["message"]["content"].asString();
                    return content;
                }
            }
        }
        ERR("DeepSeekProvider sendMessage failed, failed to parse response body");
        return "";
    }
    // 发送消息，增量返回
    std::string DeepSeekProvider::seneMessageStream(const std::vector<Message> &messages,
                                                    const std::map<std::string, std::string> &requestParam,
                                                    std::function<void(const std::string &, bool)> callback)
    {
        return "";
    }
}