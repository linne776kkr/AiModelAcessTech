#pragma once
#include <ai_chat_sdk/ChatSDK.h>
#include <ai_chat_sdk/util/myLog.h>
#include <cinttypes>
#include <httplib.h>

namespace ai_chat_server
{
    struct ChatServerConfig
    {
        //服务器配置信息
        std::string host = "0.0.0.0";//服务器绑定ip
        int port = 8080;//服务器绑定端口
        std::string logLevel = "INFO";//日志级别
        //模型需要的配置信息
        double temperature = 0.7;//温度参数
        int maxTokens = 1024;//最大token数
        //APIKey
        std::string deepSeekAPIKey;
        std::string chatGPTAPIKey;
        //ollama
        std::string ollamaModelName;//ollama模型名称
        std::string ollamaModelDesc;//ollama模型描述
        std::string ollamaEndpoint;//ollama模型endpoint
    };

    class ChatServer
    {
    public:
        ChatServer(const ChatServerConfig& config);
        ~ChatServer();

        bool start();//启动服务器
        bool stop();//停止服务器
        bool isRunning() const;//是否正在运行
    private:
        //处理创建会话请求
        void handleCreateSessionRequest(const httplib::Request& req, httplib::Response& res);
        //处理获取会话列表请求
        void handleGetSessionListRequest(const httplib::Request& req, httplib::Response& res);
        //处理获取模型列表请求
        void handleGetModelListRequest(const httplib::Request& req, httplib::Response& res);
        //处理删除会话请求
        void handleDeleteSessionRequest(const httplib::Request& req, httplib::Response& res);
        //处理获取历史消息请求
        void handleGetHistoryRequest(const httplib::Request& req, httplib::Response& res);
        //处理发送消息请求-全量返回
        void handleSendMessageRequest(const httplib::Request& req, httplib::Response& res);
        //处理发送消息请求-流式返回
        void handleSendMessageStreamRequest(const httplib::Request& req, httplib::Response& res);
        //构造响应
        std::string buildResponse(const std::string& errorMsg,bool success = false);
        //路由绑定
        void setHttpRoutes();
    private:
        ChatServerConfig _config;//服务器配置信息
        std::atomic<bool> _running = {false};//是否正在运行
        std::unique_ptr<httplib::Server> _chatServer = nullptr;     // 聊天服务器
        std::unique_ptr<ai_chat_sdk::ChatSDK> _chatSDK = nullptr;   // 聊天SDK
    };
}// namespace ai_chat_server
