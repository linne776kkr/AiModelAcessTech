#include "../include/OllamaProvider.h"
#include <jsoncpp/json/json.h>
#include <httplib.h>

namespace ai_chat_sdk
{
    bool OllamaProvider::initModel(const std::map<std::string, std::string> &modelConfig)
    {
        auto it = modelConfig.find("modelName");
        if (it == modelConfig.end())
        {
            ERR("OllamaProvider initModel falid,missing modelName in modelConfig");
            _isAvailable = false;
            return false;
        }
        _modelName = it->second;
        it = modelConfig.find("modelDesc");
        if (it == modelConfig.end())
        {
            ERR("OllamaProvider initModel failed, missing modelDesc in modelConfig");
            _isAvailable = false;
            return false;
        }
        _modelDesc = it->second;
        it = modelConfig.find("endpoint");
        if (it == modelConfig.end())
        {
            ERR("OllamaProvider initModel failed, missing endpoint in modelConfig");
            _isAvailable = false;
            return false;
        }
        _endpoint = it->second;
        _isAvailable = true;
        // 打印modelName和modelDesc和endpoint
        INFO("OllamaProvider initModel success, modelName:{}, modelDesc:{}, endpoint:{}", _modelName, _modelDesc, _endpoint);
        return true;
    }
    bool OllamaProvider::isAvailable() const
    {
        return _isAvailable;
    }
    std::string OllamaProvider::getModelName() const
    {
        return _modelName;
    }
    std::string OllamaProvider::getModelDesc() const
    {
        return _modelDesc;
    }
    std::string OllamaProvider::sendMessage(const std::vector<std::shared_ptr<Message>> &messages, const std::map<std::string, std::string> &requestParam)
    {
        // 1.检查模型是否可用
        if (_isAvailable == false)
        {
            ERR("OllamaProvider sendMessage failed, model is not available");
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
            messageJson["role"] = message->_role;
            messageJson["content"] = message->_content;
            messageArray.append(messageJson);
        }
        // 3.构造请求体
        Json::Value options;
        options["temperature"] = temperature;
        options["max_tokens"] = maxTokens;
        Json::Value requestBody;
        requestBody["model"] = getModelName();
        requestBody["messages"] = messageArray;
        requestBody["options"] = options;
        requestBody["stream"] = false; 
        // 4.进行序列化
        Json::StreamWriterBuilder writerBuilder;
        writerBuilder["indentation"] = ""; // 去除缩进和换行
        std::string requestBodyStr = Json::writeString(writerBuilder, requestBody);
        INFO("OllamaProvider sendMessage request body:{}", requestBodyStr);
        // 5.使用cpp-httplib构建HTTP客户端
        httplib::Client client(_endpoint.c_str());
        client.set_connection_timeout(30, 0); // 设置连接超时时间为30秒
        client.set_read_timeout(60, 0);       // 设置读取超时时间为60秒
        //设置请求头
        httplib::Headers headers = {
            {"Content-Type", "application/json"}};
        // 6.创建请求对象
        httplib::Request req;
        req.method = "POST";
        req.path = "/api/chat";
        req.headers = headers;
        req.body = requestBodyStr;
        //7.发送请求
        auto response = client.send(req);
        if (!response)
        {
            ERR("OllamaProvider sendMessage failed, http request failed: {}", httplib::to_string(response.error()));
            return "";
        }
        if (response->status != 200)
        {
            ERR("OllamaProvider sendMessage failed, http response status code:{}", response->status);
            return "";
        }
        // 8.解析响应体
        INFO("OllamaProvider sendMessage response body:{}", response->body);
        Json::CharReaderBuilder readerBuilder;
        std::istringstream responseStream(response->body);
        Json::Value responseBody;
        std::string responseError;
        // 反序列化响应体，获取content字段的值并返回
        if (Json::parseFromStream(readerBuilder, responseStream, &responseBody, &responseError))
        {
            if(responseBody.isMember("message")&&responseBody["message"].isMember("content")){
                std::string content = responseBody["message"]["content"].asString();
                INFO("OllamaProvider sendMessage response body:{}", content);
                return content;
            }
        }
        //序列化解析失败
        ERR("OllamaProvider sendMessage failed, parse response body failed:{}", responseError);
        return "";
    }
    std::string OllamaProvider::sendMessageStream(const std::vector<std::shared_ptr<Message>> &messages,
                                                  const std::map<std::string, std::string> &requestParam,
                                                  std::function<void(const std::string &, bool)> callback)
    {
        // 1.检查模型是否可用
        if (_isAvailable == false)
        {
            ERR("OllamaProvider sendMessage failed, model is not available");
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
            messageJson["role"] = message->_role;
            messageJson["content"] = message->_content;
            messageArray.append(messageJson);
        }
        // 3.构造请求体
        Json::Value options;
        options["temperature"] = temperature;
        options["max_tokens"] = maxTokens;
        Json::Value requestBody;
        requestBody["model"] = getModelName();
        requestBody["messages"] = messageArray;
        requestBody["options"] = options;
        requestBody["stream"] = true; // 开启流式返回
        // 4.进行序列化
        Json::StreamWriterBuilder writerBuilder;
        writerBuilder["indentation"] = ""; // 去除缩进和换行
        std::string requestBodyStr = Json::writeString(writerBuilder, requestBody);
        INFO("OllamaProvider sendMessage request body:{}", requestBodyStr);
        // 5.使用cpp-httplib构建HTTP客户端
        httplib::Client client(_endpoint.c_str());
        client.set_connection_timeout(60, 0); // 设置连接超时时间为60秒
        client.set_read_timeout(300, 0);       // 设置读取超时时间为300秒
        //设置请求头
        httplib::Headers headers = {
            {"Content-Type", "application/json"}};
        // 流式处理变量
        std::string buffer;        // 接收流式返回的数据
        bool gotError = false;     // 响应是否成功
        bool streamFinish = false; // 流式返回是否结束
        std::string fullResponse;  // 完整的响应内容
        // 6.创建请求对象
        httplib::Request req;
        req.method = "POST";
        req.path = "/api/chat";
        req.headers = headers;
        req.body = requestBodyStr;
        //设置响应处理器
        req.response_handler = [&](const httplib::Response &response)
        {
            if (response.status != 200)
            {
                gotError = true;
                ERR("OllamaProvider sendMessage failed, http response status code:{}", response.status);
                return false;
            }
            return true;
        };
        //设置流式返回处理器
        req.content_receiver = [&](const char *data, size_t data_length, size_t offset, size_t total_length)
        {
            // 验证响应头是否错误
            if (gotError)
            {
                return false;
            }
            // 将接收到的数据追加到buffer中
            buffer.append(data, data_length);
            //返回的数据以/n换行符分隔
            size_t pos = 0;
            while ((pos = buffer.find("\n")) != std::string::npos)
            {
                // 从buffer中提取一个数据块
                std::string chunk = buffer.substr(0, pos);
                buffer.erase(0, pos + 1);
                INFO("OllamaProvider sendMessageStream received chunk: {}", chunk);
                 // 解析json字符串，获取增量内容
                Json::CharReaderBuilder readerBuilder;
                std::istringstream responseStream(chunk);
                Json::Value responseBody;
                std::string responseError;
                if (Json::parseFromStream(readerBuilder, responseStream, &responseBody, &responseError))
                {
                    if(responseBody.isMember("done")&&responseBody["done"]==true)
                    {
                        streamFinish = true;
                        callback("", true);
                        return true;
                    }
                    // 判断响应体中是否有message字段，并且message字段是一个非空数组
                    if (responseBody.isMember("message") && responseBody["message"].isMember("content"))
                    {
                        // 获取content字段的值，并将其作为增量内容返回给调用方
                        std::string content = responseBody["message"]["content"].asString();
                        fullResponse += content;
                        callback(content, false);
                        continue;
                    }
                }
                WARN("OllamaProvider sendMessageStream failed to parse chunk json: {}, error: {}", chunk, responseError);
                continue;
            }
            return true;
        };
        // 7.发送http请求
        auto request = client.send(req);
        if (!request)
        {
            // 请求发送失败
            ERR("OllamaProvider sendMessageStream failed, http request failed: {}", httplib::to_string(request.error()));
            return "";
        }
        if (!streamFinish)
        {
            // 流式返回异常结束
            WARN("OllamaProvider sendMessageStream failed, stream not finish");
            callback("", true);
            return "";
        }
        INFO("OllamaProvider sendMessageStream success, full response:{}", fullResponse);
        return fullResponse;
    }
} // end namespace