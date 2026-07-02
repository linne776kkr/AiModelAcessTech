#include "ChatServer.h"
#include <jsoncpp/json/json.h>

namespace ai_chat_server
{
    ChatServer::ChatServer(const ChatServerConfig& config)
    {
        // 初始化聊天SDK
        std::vector<std::shared_ptr<ai_chat_sdk::Config>> configs;
        //构造deepseek模型配置参数
        std::shared_ptr<ai_chat_sdk::ApiConfig> deepseekConfig = std::make_shared<ai_chat_sdk::ApiConfig>();
        deepseekConfig->_modelName = "deepseek-chat";
        deepseekConfig->_temperature = config.temperature;
        deepseekConfig->_maxTokens = config.maxTokens;
        deepseekConfig->_apiKey = config.deepSeekAPIKey;
        configs.push_back(deepseekConfig);
        //构建chatGPT模型配置参数
        std::shared_ptr<ai_chat_sdk::ApiConfig> gpt4oConfig = std::make_shared<ai_chat_sdk::ApiConfig>();
        gpt4oConfig->_modelName = "gpt-4o-mini";
        gpt4oConfig->_temperature = config.temperature;
        gpt4oConfig->_maxTokens = config.maxTokens;
        gpt4oConfig->_apiKey = config.chatGPTAPIKey;
        configs.push_back(gpt4oConfig);
        //构建ollama模型配置参数
        std::shared_ptr<ai_chat_sdk::OllamaConfig> ollamaConfig = std::make_shared<ai_chat_sdk::OllamaConfig>();
        ollamaConfig->_modelName = config.ollamaModelName;
        ollamaConfig->_temperature = config.temperature;
        ollamaConfig->_maxTokens = config.maxTokens;
        ollamaConfig->_modelDesc = config.ollamaModelDesc;
        ollamaConfig->_endpoint = config.ollamaEndpoint;
        configs.push_back(ollamaConfig);
        _chatSDK = std::make_unique<ai_chat_sdk::ChatSDK>();
        if(!_chatSDK->initModel(configs))
        {
            ERR("initModel failed");
            return;
        }
        // 初始化聊天服务器
        _chatServer = std::make_unique<httplib::Server>();
        if(!_chatServer)
        {
            ERR("create server failed");
            return;
        }
        INFO("create server success");
        return;
    }

    ChatServer::~ChatServer()
    {
        stop();
    }

    bool ChatServer::start()
    {
        if(_running.load())
        {
            ERR("server is running");
            return false;
        }
        setHttpRoutes();
        //启动聊天服务器,开启一个线程,服务器在单独的线程中运行
        std::thread serverThread([this](){
            _chatServer->listen(_config.host,_config.port);
            INFO("ChatServer start on {}:{}",_config.host,_config.port);
        });
        //分离线程,避免阻塞主线程
        serverThread.detach();
        _running.store(true);
        INFO("server start success");
        return true;
    }
    bool ChatServer::stop()
    {
        if(!_running.load())
        {
            ERR("server is not running");
            return false;
        }
        //停止聊天服务器
        if(_chatServer)_chatServer->stop();
        _running.store(false);
        INFO("server stop success");
        return true;
    }
    bool ChatServer::isRunning() const
    {
        return _running.load();
    }

    //处理创建会话请求
    void ChatServer::handleCreateSessionRequest(const httplib::Request& req, httplib::Response& res)
    {
        //解析请求体,反序列化为Json对象
        Json::CharReaderBuilder readerBuilder;
        std::istringstream responseStream(req.body);
        Json::Value responseBody;
        std::string responseError;
        if( !Json::parseFromStream(readerBuilder, responseStream, &responseBody, &responseError)||
            !responseBody.isMember("model"))
        {
            std::string errorJsonStr = buildResponse(responseError,false);
            res.set_content(errorJsonStr, "application/json");
            res.status = 400;//客户端发送的请求有错误,服务器无法解析
            return;
        }
        //创建会话
        std::string modelName = responseBody["model"].asString();
        std::string sessionId = _chatSDK->createSession(modelName);
        if(sessionId.empty())
        {
            std::string errorJsonStr = buildResponse("create session failed",false);
            res.set_content(errorJsonStr, "application/json");
            res.status = 500;//服务器内部错误
            return;
        }
        //构建响应参数
        Json::Value dataJson;
        dataJson["session_id"] = sessionId;
        dataJson["model_name"] = modelName;
        Json::Value responseJson;
        responseJson["success"] = true;
        responseJson["message"] = "create session success";
        responseJson["data"] = dataJson;
        //序列化
        Json::StreamWriterBuilder writerBuilder;
        writerBuilder["indentation"] = ""; // 去除缩进和换行
        std::string responseJsonStr = Json::writeString(writerBuilder, responseJson);
        res.set_content(responseJsonStr, "application/json");
        res.status = 200;//请求成功
        return;
    }
    //处理获取会话列表请求
    void ChatServer::handleGetSessionListRequest(const httplib::Request& req, httplib::Response& res)
    {
        //获取会话列表
        std::vector<std::string>  sessionLists = _chatSDK->getAllSessionLists();
        //构建响应参数
        Json::Value dataArray(Json::arrayValue);
        for(const auto& sessionId : sessionLists)
        {
            //根据sessionId获取指定会话
            const std::shared_ptr<ai_chat_sdk::Session> session = _chatSDK->getSession(sessionId);
            if(!session)
            {
                ERR("getSession session failed, sessionId: {}",sessionId);
                continue;
            }
            //构建会话信息Json对象
            Json::Value sessionJson;
            sessionJson["id"] = sessionId;
            sessionJson["name"] = session->_sessionName;
            sessionJson["model"] = session->_modelName;
            sessionJson["created_at"] = static_cast<int64_t>(session->_createTime);
            sessionJson["updated_at"] = static_cast<int64_t>(session->_lastUpdateTime);
            sessionJson["message_count"] = session->_messages.size();
            dataArray.append(sessionJson);
        }
        //构建响应体
        Json::Value responseJson;
        responseJson["success"] = true;
        responseJson["message"] = "get session list success";
        responseJson["data"] = dataArray;
        //序列化
        Json::StreamWriterBuilder writerBuilder;
        writerBuilder["indentation"] = ""; //去除缩进和换行
        std::string responseJsonStr = Json::writeString(writerBuilder,responseJson);
        res.set_content(responseJsonStr,"application/json");
        res.status = 200;//成功
        return;
    }
    //处理获取模型列表请求
    void ChatServer::handleGetModelListRequest(const httplib::Request& req, httplib::Response& res)
    {
        //获取可用模型信息
        std::vector<std::shared_ptr<ai_chat_sdk::ModelInfo>> models = _chatSDK->getAvailableModels();
        //构建响应参数
        Json::Value dataArray(Json::arrayValue);
        for(const auto& model :models)
        {
            Json::Value modelJson;
            modelJson["name"] = model->_modelName;
            modelJson["desc"] = model->_modelDesc;
            dataArray.append(modelJson);
        }
        //构建响应体
        Json::Value responseJson;
        responseJson["success"] = true;
        responseJson["message"] = "get model list success";
        responseJson["data"] = dataArray;
        //序列化
        Json::StreamWriterBuilder writerBuilder;
        writerBuilder["indentation"] = "";//去除缩进和换行
        std::string responseJsonStr = Json::writeString(writerBuilder,responseJson);
        res.set_content(responseJsonStr,"application/json");
        res.status = 200;//成功
        INFO("get model list success, model count: {}",models.size());
        return;
    }
    //处理删除会话请求
    void ChatServer::handleDeleteSessionRequest(const httplib::Request& req, httplib::Response& res)
    {
        //获取会话参数,是个路径参数
        std::string sessionId = req.path_params.at("session_id");
        //根据会话id删除会话
        if(!_chatSDK->deleteSession(sessionId))
        {
            std::string errorJsonStr = buildResponse("delete session failed",false);
            res.set_content(errorJsonStr,"application/json");
            res.status = 404;//会话不存在或已被删除
            return;
        }
        std::string responseJsonStr = buildResponse("delete session success",true);
        res.set_content(responseJsonStr,"application/json");
        res.status = 200;//成功
        INFO("delete session success, sessionId: {}",sessionId);
        return;
    }
    //处理获取历史消息请求
    void ChatServer::handleGetHistoryRequest(const httplib::Request& req, httplib::Response& res)
    {
        //获取会话id
        std::string sessionId = req.path_params.at("session_id");
        //根据会话id获取会话
        const std::shared_ptr<ai_chat_sdk::Session> session = _chatSDK->getSession(sessionId);
        if(!session)
        {
            std::string errorJsonStr = buildResponse("get session history failed",false);
            res.set_content(errorJsonStr,"application/json");
            res.status = 404;//会话不存在
            return;
        }
        //构建响应参数
        Json::Value dataArray(Json::arrayValue);
        for(const auto& message : session->_messages)
        {
            Json::Value messageJson;
            messageJson["id"] = message->_messageId;
            messageJson["role"] = message->_role;
            messageJson["content"] = message->_content;
            messageJson["timestamp"] = static_cast<int64_t>(message->_timestamp);
            dataArray.append(messageJson);
        }
        //构建响应体
        Json::Value responseJson;
        responseJson["success"] = true;
        responseJson["message"] = "get session history success";
        responseJson["data"] = dataArray;
        //序列化
        Json::StreamWriterBuilder writerBuilder;
        writerBuilder["indentation"] = ""; //去除缩进和换行
        std::string responseJsonStr = Json::writeString(writerBuilder,responseJson);
        res.set_content(responseJsonStr,"application/json");
        res.status = 200;//成功
        INFO("get session history success, sessionId: {}, message count: {}",sessionId,session->_messages.size());
        return;
    }
    //处理发送消息请求-全量返回
    void ChatServer::handleSendMessageRequest(const httplib::Request& req, httplib::Response& res)
    {
        //解析请求体,反序列化为Json对象
        Json::CharReaderBuilder readerBuilder;
        std::istringstream responseStream(req.body);
        Json::Value responseBody;
        std::string responseError;
        if( !Json::parseFromStream(readerBuilder, responseStream, &responseBody, &responseError)||
            !responseBody.isMember("session_id")||
            !responseBody.isMember("message"))
        {
            std::string errorJsonStr = buildResponse(responseError,false);
            res.set_content(errorJsonStr, "application/json");
            res.status = 400;//客户端发送的请求有错误,服务器无法解析
            return;
        }
        //发送消息
        std::string sessionId = responseBody["session_id"].asString();
        std::string message = responseBody["message"].asString();
        std::string response = _chatSDK->sendMessage(sessionId, message);
        if(response.empty())
        {
            std::string errorJsonStr = buildResponse("send message failed",false);
            res.set_content(errorJsonStr, "application/json");
            res.status = 500;//服务器内部错误
            return;
        }
        //构建响应参数
        Json::Value dataJson;
        dataJson["session_id"] = sessionId;
        dataJson["response"] = response;
        //构建响应体
        Json::Value responseJson;
        responseJson["success"] = true;
        responseJson["message"] = "send message success";
        responseJson["data"] = dataJson;
        //序列化
        Json::StreamWriterBuilder writerBuilder;
        writerBuilder["indentation"] = ""; // 去除缩进和换行
        std::string responseJsonStr = Json::writeString(writerBuilder, responseJson);
        res.set_content(responseJsonStr,"application/json");
        res.status = 200;//成功
        return;
    }
    //处理发送消息请求-流式返回
    void ChatServer::handleSendMessageStreamRequest(const httplib::Request& req, httplib::Response& res)
    {
        //解析请求体,反序列化为Json对象
        Json::CharReaderBuilder readerBuilder;
        std::istringstream responseStream(req.body);
        Json::Value responseBody;
        std::string responseError;
        if( !Json::parseFromStream(readerBuilder, responseStream, &responseBody, &responseError)||
            !responseBody.isMember("session_id")||
            !responseBody.isMember("message"))
        {
            std::string errorJsonStr = buildResponse(responseError,false);
            res.set_content(errorJsonStr, "application/json");
            res.status = 400;//客户端发送的请求有错误,服务器无法解析
            return;
        }
        std::string sessionId = responseBody["session_id"].asString();
        std::string message = responseBody["message"].asString();
        //准备流式响应
        res.status = 200;//成功
        res.set_header("Content-Type", "text/event-stream");      // SSE 标准类型
        res.set_header("Cache-Control", "no-cache");              // 禁用缓存
        res.set_header("Connection", "keep-alive");               // 保持连接
        //res.set_header("Content-Type","no-cache");//不使用缓存,服务器立即将数据发送到网络
        //res.set_header("Connection","keep-alive");//保持连接,服务器不会关闭请求
        //设置了这个函数,当所在函数退出时,httplib会不断调用设置的回调函数,直到返回false
        //第一个参数是偏移量,第二个参数是数据接收器
        res.set_chunked_content_provider("text/event-stream",
            [this,sessionId,message](size_t offset, httplib::DataSink &sink)->bool{
            auto writeChunk = [&](const std::string& chunk,bool last){
                //将chunk转换为SSE数据格式
                //Json::valueToQuotedString将Json字符串转换为双引号字符串,避免Json字符串中包含引号导致解析错误
                std::string sseData = "data: "+Json::valueToQuotedString(chunk.c_str())+"\n\n";
                //将结果写入sink
                sink.write(sseData.c_str(),sseData.size());
                //处理结束标志
                if(last)
                {
                    std::string doneData = "data: [DONE]\n\n";
                    sink.write(doneData.c_str(),doneData.size());
                    sink.done();
                }
            };
            //先发送一个空数据,通知客户端开始接收数据
            writeChunk("",false);
            _chatSDK->sendStreamMessage(sessionId,message,writeChunk);
            return false;
        });
    }
    //构造响应
    std::string ChatServer::buildResponse(const std::string& errorMsg,bool success)
    {
        //构建响应参数
        Json::Value errorJson;
        errorJson["success"] = success;
        errorJson["message"] = errorMsg;
        //序列化
        Json::StreamWriterBuilder writerBuilder;
        writerBuilder["indentation"] = ""; // 去除缩进和换行
        std::string errorJsonStr = Json::writeString(writerBuilder, errorJson);
        return errorJsonStr;
    }
    //路由绑定
    void ChatServer::setHttpRoutes()
    {
    // 处理创建会话请求
    _chatServer->Post("/api/session",[this](const httplib::Request& request, httplib::Response& response){
    handleCreateSessionRequest(request,response);});
    // 处理获取会话列表请求
    _chatServer->Get("/api/sessions",[this](const httplib::Request& request, httplib::Response& response){
    handleGetSessionListRequest(request,response);});

    // 处理获取模型列表请求
    _chatServer->Get("/api/models",[this](const httplib::Request& request, httplib::Response& response){
    handleGetModelListRequest(request,response);});

    // 处理删除会话请求
    _chatServer->Delete("/api/session/:session_id",[this](const httplib::Request& request, httplib::Response& response){
    handleDeleteSessionRequest(request,response);});    

    // 处理获取历史消息请求
    _chatServer->Get("/api/session/:session_id/history",[this](const httplib::Request& request, httplib::Response& response){
    handleGetHistoryRequest(request,response);});

    // 处理发送消息请求-全部返回
    _chatServer->Post("/api/message",[this](const httplib::Request& request, httplib::Response& response){
    handleSendMessageRequest(request,response);});

    // 处理发送消息请求-增量返回
    _chatServer->Post("/api/message/async",[this](const httplib::Request& request, httplib::Response& response){
    handleSendMessageStreamRequest(request,response);});

    // 静态文件服务
    _chatServer->set_mount_point("/", "./www");
    _chatServer->Get("/", [](const httplib::Request& req, httplib::Response& res) {
        res.set_redirect("/index.html");
    });
    }
}// namespace ai_chat_server