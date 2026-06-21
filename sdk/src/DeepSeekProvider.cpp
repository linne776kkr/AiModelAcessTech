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
        // 打印apikey和endpoint
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

        // 6.创建请求对象
        httplib::Request req;
        req.method = "POST";
        req.path = "/v1/chat/completions";
        req.headers = headers;
        req.body = requestBodyStr;
        // 7.发送http请求
        auto request = client.send(req);
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
        // 8.解析响应体
        Json::CharReaderBuilder readerBuilder;
        std::istringstream responseStream(request->body);
        Json::Value responseBody;
        std::string responseError;
        // 反序列化响应体，获取content字段的值并返回
        if (Json::parseFromStream(readerBuilder, responseStream, &responseBody, &responseError))
        {
            // 判断响应体中是否有choices字段，并且choices字段是一个非空数组
            if (responseBody.isMember("choices") && responseBody["choices"].isArray() && !responseBody["choices"].empty())
            {
                // 从choices数组中获取第一个元素，并判断该元素是否有message字段，并且message字段中是否有content字段
                auto choice = responseBody["choices"][0];
                if (choice.isMember("message") && choice["message"].isMember("content"))
                {
                    // 获取content字段的值，并将其作为函数的返回值
                    std::string content = choice["message"]["content"].asString();
                    return content;
                }
            }
        }
        // responseBody反序列化失败
        // 走到这里说明反序列化解析失败
        ERR("DeepSeekProvider sendMessage failed, failed to parse response body, error: {}", responseError);
        return "";
    }
    // 发送消息，增量返回
    std::string DeepSeekProvider::sendMessageStream(const std::vector<Message> &messages,
                                                    const std::map<std::string, std::string> &requestParam,
                                                    std::function<void(const std::string &, bool)> callback)
    {
        // 1.检查模型是否可用
        if (_isAvailable == false)
        {
            ERR("DeepSeekProvider seneMessageStream failed, model is not available");
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
        requestBody["stream"] = true; // 开启流式返回
        // 4.进行序列化
        Json::StreamWriterBuilder writerBuilder;
        writerBuilder["indentation"] = ""; // 去除缩进和换行
        std::string requestBodyStr = Json::writeString(writerBuilder, requestBody);
        INFO("DeepSeekProvider seneMessageStream request body:{}", requestBodyStr);
        // 5.使用cpp-httplib构建HTTP客户端
        httplib::Client client(_endpoint.c_str());
        client.set_connection_timeout(30, 0); // 设置连接超时时间为30秒
        client.set_read_timeout(300, 0);      // 设置读取超时时间为5分钟
        // 设置请求头,apikey认证方法,序列化格式,以及开启流式返回需要设置Accept为text/event-stream
        httplib::Headers headers = {
            {"Authorization", "Bearer " + _apiKey},
            {"Content-Type", "application/json"},
            {"Accept", "text/event-stream"}};
        // 流式处理变量
        std::string buffer;        // 接收流式返回的数据
        bool gotError = false;     // 响应是否成功
        bool streamFinish = false; // 流式返回是否结束
        std::string fullResponse;  // 完整的响应内容
        // 6.创建请求对象
        httplib::Request req;
        req.method = "POST";
        req.path = "/v1/chat/completions";
        req.headers = headers;
        req.body = requestBodyStr;
        // 设置响应处理器
        req.response_handler = [&](const httplib::Response &response)
        {
            if (response.status != 200)
            {
                ERR("DeepSeekProvider seneMessageStream failed, http response status code:{}", response.status);
                gotError = true;
                return false;
            }
            return true;
        };
        req.content_receiver = [&](const char *data, size_t data_length, size_t offset, size_t total_length)
        {
            // 验证响应头是否错误
            if (gotError)
            {
                return false;
            }
            // 将接收到的数据追加到buffer中
            buffer.append(data, data_length);
            // 处理流式响应的数据块，流式响应的数据块以\n\n分隔
            size_t pos = 0;
            while ((pos = buffer.find("\n\n")) != std::string::npos)
            {
                // 从buffer中提取一个数据块
                std::string chunk = buffer.substr(0, pos);
                buffer.erase(0, pos + 2);
                INFO("DeepSeekProvider seneMessageStream received chunk: {}", chunk);
                // 解析该数据块中的数据，流式响应的数据块以data:开头，后面跟着一个json字符串
                // 处理空行和注释行
                if (chunk.empty() || chunk[0] == ':')
                {
                    continue;
                }
                // 获取data:后面的json字符串
                if (chunk.compare(0, 6, "data: ") == 0)
                {
                    std::string jsonStr = chunk.substr(6);
                    // 检测是否为结束标志
                    if (jsonStr == "[DONE]")
                    {
                        streamFinish = true;
                        callback("", true);
                        return true;
                    }
                    // 解析json字符串，获取增量内容
                    Json::CharReaderBuilder readerBuilder;
                    std::istringstream responseStream(jsonStr);
                    Json::Value responseBody;
                    std::string responseError;
                    if (Json::parseFromStream(readerBuilder, responseStream, &responseBody, &responseError))
                    {
                        // 判断响应体中是否有choices字段，并且choices字段是一个非空数组
                        if (responseBody.isMember("choices") && responseBody["choices"].isArray() && !responseBody["choices"].empty())
                        {
                            // 从choices数组中获取第一个元素，并判断该元素是否有message字段，并且message字段中是否有content字段
                            auto choice = responseBody["choices"][0];
                            if (choice.isMember("delta") && choice["delta"].isMember("content"))
                            {
                                // 获取content字段的值，并将其作为增量内容返回给调用方
                                std::string content = choice["delta"]["content"].asString();
                                fullResponse += content;
                                callback(content, false);
                                continue;
                            }
                        }
                    }
                    // responseBody反序列化失败
                    WARN("DeepSeekProvider seneMessageStream failed to parse chunk json: {}, error: {}", jsonStr, responseError);
                }
                else
                {
                    // 数据块格式错误，既不是空行，也不是以data:开头的json字符串
                    WARN("DeepSeekProvider seneMessageStream failed, invalid chunk format:{}", chunk);
                }
            }
            return true;
        };
        // 7.发送http请求
        auto request = client.send(req);
        if (!request)
        {
            // 请求发送失败
            ERR("DeepSeekProvider sendMessageStream failed, http request failed: {}", httplib::to_string(request.error()));
            return "";
        }
        if (!streamFinish)
        {
            // 流式返回异常结束
            WARN("DeepSeekProvider seneMessageStream failed, stream not finish");
            callback("", true);
            return "";
        }
        INFO("DeepSeekProvider seneMessageStream success, full response:{}", fullResponse);
        return fullResponse;
    }
}//end namespace ai_chat_sdk