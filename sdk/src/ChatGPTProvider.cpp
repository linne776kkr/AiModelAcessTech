#include "../include/ChatGPTProvider.h"
#include <jsoncpp/json/json.h>
#include <httplib.h>

namespace ai_chat_sdk
{
    // 初始化模型，传入模型配置信息
    bool ChatGPTProvider::initModel(const std::map<std::string, std::string> &modelConfig)
    {
        auto it = modelConfig.find("apiKey");
        if (it == modelConfig.end())
        {
            ERR("ChatGPTProvider initModel failed, missing apiKey in modelConfig");
            _isAvailable = false;
            return false;
        }
        _apiKey = it->second;
        it = modelConfig.find("endpoint");
        if (it == modelConfig.end())
        {
            ERR("ChatGPTProvider initModel failed, missing endpoint in modelConfig");
            _isAvailable = false;
            return false;
        }
        _endpoint = it->second;
        _isAvailable = true;
        // 打印apikey和endpoint
        INFO("ChatGPTProvider initModel success, apiKey:{}, endpoint:{}", _apiKey, _endpoint);
        return true;
    }
    // 检测模型是否有效
    bool ChatGPTProvider::isAvailable() const
    {
        return _isAvailable;
    }
    // 获取模型名称
    std::string ChatGPTProvider::getModelName() const
    {
        return "gpt-4o-mini";
    }
    // 获取模型描述信息
    std::string ChatGPTProvider::getModelDesc() const
    {
        return "OpenAI推出的轻量级,高性价比模型,适用于对话场景,支持增量返回,接口兼容ChatGPT-4.0";
    }
    // 发送消息,全量返回
    std::string ChatGPTProvider::sendMessage(const std::vector<Message> &messages, const std::map<std::string, std::string> &requestParam)
    {
        // 1.检查模型是否可用
        if (_isAvailable == false)
        {
            ERR("ChatGPTProvider sendMessage failed, model is not available");
            return "";
        }
        // 2.构建请求参数
        double temperature = 0.7;
        int maxTokens = 2048;
        auto it = requestParam.find("temperature");
        if (it != requestParam.end())
        {
            temperature = std::stod(it->second);
        }
        // 支持两种参数命名方式，max_input_tokens和maxTokens,优先使用max_input_tokens
        it = requestParam.find("max_input_tokens");
        if (it != requestParam.end())
        {
            maxTokens = std::stoi(it->second);
        }
        else
        {
            it = requestParam.find("maxTokens");
            if (it != requestParam.end())
            {
                maxTokens = std::stoi(it->second);
            }
        }
        // 构建历史消息
        Json::Value messageArray(Json::arrayValue);
        for (const auto &message : messages)
        {
            Json::Value messageJson;
            messageJson["role"] = message._role;
            messageJson["content"] = message._content;
            messageArray.append(messageJson);
        }
        // 3.构建请求体
        Json::Value requestBody;
        requestBody["model"] = getModelName();
        requestBody["input"] = messageArray;
        requestBody["temperature"] = temperature;
        requestBody["max_output_tokens"] = maxTokens;
        // 4.进行序列化
        Json::StreamWriterBuilder writerBuilder;
        writerBuilder["indentation"] = ""; // 去除json字符串中的空格和换行
        std::string requestBodyStr = Json::writeString(writerBuilder, requestBody);
        INFO("ChatGPTProvider sendMessage request body:{}", requestBodyStr);
        // 5.使用cpp-httplib构建HTTP客户端
        httplib::Client client(_endpoint.c_str());
        client.set_connection_timeout(30, 0); // 设置连接超时时间为30秒
        client.set_read_timeout(60, 0);       // 设置读取超时时间为60秒
        client.set_proxy("127.0.0.1", 7890);  // 设置代理
        // 设置请求头
        httplib::Headers headers = {
            {"Authorization", "Bearer " + _apiKey},
            {"Content-Type", "application/json"}};
        // 6.创建请求对象
        httplib::Request req;
        req.method = "POST";
        req.path = "/v1/responses";
        req.headers = headers;
        req.body = requestBodyStr;
        // 7.发送http请求
        auto request = client.send(req);
        if (!request)
        {
            ERR("ChatGPTProvider sendMessage failed, http request failed");
            return "";
        }
        if (request->status != 200)
        {
            ERR("ChatGPTProvider sendMessage failed, http response status: {}", request->status);
        }
        INFO("ChatGPTProvider sendMessage success, response body:{}", request->body);
        INFO("ChatGPTProvider sendMessage success, response status code:{}", request->status);
        // 8.解析响应体
        Json::CharReaderBuilder readerBuilder;
        std::istringstream responseStream(request->body);
        Json::Value responseBody;
        std::string responseError;
        // 反序列化响应体，获取content字段的值并返回
        if (Json::parseFromStream(readerBuilder, responseStream, &responseBody, &responseError))
        {
            if (responseBody.isMember("output") && responseBody["output"].isArray() && !responseBody["output"].empty())
            {
                auto output = responseBody["output"][0];
                if (output.isMember("content") && output["content"].isArray() && !output["content"].empty())
                {
                    auto content = output["content"][0];
                    if (content.isMember("text"))
                    {
                        std::string text = content["text"].asString();
                        INFO("ChatGPTProvider sendMessage success, response content: {}", text);
                        return text;
                    }
                }
            }
        }
        ERR("ChatGPTProvider sendMessage failed, failed to parse response json: {}, error: {}", request->body, responseError);
        return "";
        return "";
    }
    // 发送消息，增量返回
    std::string ChatGPTProvider::sendMessageStream(const std::vector<Message> &messages,
                                                   const std::map<std::string, std::string> &requestParam,
                                                   std::function<void(const std::string &, bool)> callback)
    {
        // 1.检查模型是否可用
        if (_isAvailable == false)
        {
            ERR("ChatGPTProvider sendMessageStream failed, model is not available");
            return "";
        }
        // 2.构建请求参数
        double temperature = 0.7;
        int maxTokens = 2048;
        auto it = requestParam.find("temperature");
        if (it != requestParam.end())
        {
            temperature = std::stod(it->second);
        }
        // 支持两种参数命名方式，max_input_tokens和maxTokens,优先使用max_input_tokens
        it = requestParam.find("max_input_tokens");
        if (it != requestParam.end())
        {
            maxTokens = std::stoi(it->second);
        }
        else
        {
            it = requestParam.find("maxTokens");
            if (it != requestParam.end())
            {
                maxTokens = std::stoi(it->second);
            }
        }
        // 构建历史消息
        Json::Value messageArray(Json::arrayValue);
        for (const auto &message : messages)
        {
            Json::Value messageJson;
            messageJson["role"] = message._role;
            messageJson["content"] = message._content;
            messageArray.append(messageJson);
        }
        // 3.构建请求体
        Json::Value requestBody;
        requestBody["model"] = getModelName();
        requestBody["input"] = messageArray;
        requestBody["temperature"] = temperature;
        requestBody["max_output_tokens"] = maxTokens;
        requestBody["stream"] = true; // 增量返回
        // 4.进行序列化
        Json::StreamWriterBuilder writerBuilder;
        writerBuilder["indentation"] = ""; // 去除json字符串中的空格和换行
        std::string requestBodyStr = Json::writeString(writerBuilder, requestBody);
        INFO("ChatGPTProvider sendMessageStream request body:{}", requestBodyStr);
        // 5.使用cpp-httplib构建HTTP客户端
        httplib::Client client(_endpoint.c_str());
        client.set_connection_timeout(60, 0); // 设置连接超时时间为60秒
        client.set_read_timeout(300, 0);      // 设置读取超时时间为300秒
        client.set_proxy("127.0.0.1", 7890); // 设置代理
        // 设置请求头
        httplib::Headers headers = {
            {"Authorization", "Bearer " + _apiKey},
            {"Content-Type", "application/json"},
            {"Accept", "text/event-stream"}}; // 设置Accept为text/event-stream，表示接受增量返回的流式响应
        // 流式处理变量
        std::string buffer;        // 接收流式返回的数据
        bool gotError = false;     // 响应是否成功
        bool streamFinish = false; // 流式返回是否结束
        std::string fullResponse;  // 完整的响应内容
        // 6.创建请求对象
        httplib::Request req;
        req.method = "POST";
        req.path = "/v1/responses";
        req.headers = headers;
        req.body = requestBodyStr;
        // 设置响应处理器
        req.response_handler = [&](const httplib::Response &response)
        {
            if (response.status != 200)
                {
                ERR("ChatGPTProvider sendMessageStream failed, http response status code:{}", response.status);
                gotError = true;
                return false; // 返回false表示停止接收响应
            }
            return true; // 返回true表示继续接收响应
        };
        // 设置增量返回处理器
        req.content_receiver = [&](const char *data, size_t data_length, size_t offset, size_t total_length)
        {
            if (gotError)
            {
                return false;
            }
            // 将接收到的数据追加到buffer中
            buffer.append(data, data_length);
            size_t pos = 0;
            while ((pos = buffer.find("\n\n")) != std::string::npos)
            {
                // 从buffer中提取一个数据块
                std::string chunk = buffer.substr(0, pos);
                buffer.erase(0, pos + 2);
                INFO("ChatGPTProvider sendMessageStream received chunk: {}", chunk);
                // 解析该数据块中的数据，流式响应的数据块以data:开头，后面跟着一个json字符串
                // 处理空行和注释行
                if (chunk.empty() || chunk[0] == ':')
                {
                    continue;
                }
                // 获取event: 后面的事件类型，然后换行data: 后面的json字符串
                if (chunk.compare(0, 7, "event: ") == 0)
                {
                    // 获取事件类型
                    size_t index = chunk.find("\n");
                    std::string eventType = chunk.substr(7, index - 7);
                    // 获取json字符串,index为换行符的位置,后面data: 后面是json字符串
                    std::string jsonStr = chunk.substr(index + 7);
                    if (eventType == "response.created")
                    {
                        continue;
                    }
                    else if (eventType == "response.in_progress")
                    {
                        continue;
                    }
                    else if (eventType == "response.output_item.added")
                    {
                        continue;
                    }
                    else if (eventType == "response.content_part.added")
                    {
                        continue;
                    }
                    else if (eventType == "response.output_text.delta")
                    {
                        // 解析json字符串，获取增量内容
                        Json::CharReaderBuilder readerBuilder;
                        std::istringstream responseStream(jsonStr);
                        Json::Value responseBody;
                        std::string responseError;
                        if (Json::parseFromStream(readerBuilder, responseStream, &responseBody, &responseError))
                        {
                            if (responseBody.isMember("delta"))
                            {
                                std::string delta = responseBody["delta"].asString();
                                fullResponse += delta;
                                callback(delta, false);
                            }
                        }
                    }
                    else if (eventType == "response.output_text.done")
                    {
                        continue;
                    }
                    else if (eventType == "response.content_part.done")
                    {
                        continue;
                    }
                    else if (eventType == "response.output_item.done")
                    {
                        continue;
                    }
                    else if (eventType == "response.completed")
                    {
                        streamFinish = true;
                        callback("", true);
                        return true;
                    }
                    else
                    {
                        WARN("ChatGPTProvider sendMessageStream received unknown event type: {}, json: {}", eventType, jsonStr);
                        continue;
                    }
                }
            }
            return true;
        };
        // 7.发送http请求
        auto request = client.send(req);
        if (!request)
        {
            // 请求发送失败
            ERR("ChatGPTProvider sendMessageStream failed, http request failed: {}", httplib::to_string(request.error()));
            return "";
        }
        if (!streamFinish)
        {
            // 流式返回异常结束
            WARN("ChatGPTProvider sendMessageStream failed, stream not response.completed");
            callback("", true);
            return "";
        }
        INFO("ChatGPTProvider sendMessageStream success, full response:{}", fullResponse);
        return fullResponse;
    }
} // end namespace ai_chat_sdk