#pragma once
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <memory>
#include <vector>
#include "DataManager.h"
#include "common.h"


namespace ai_chat_sdk{
    class SessionManager{
        public:
            //构造函数
            SessionManager(const std::string& dbPath = "ai_chat_sdk.db");
            //创建会话,提供模型名称,返回会话ID
            std::string createSession(const std::string& modelName, const std::string& sessionName = "");
            //获取指定会话
            const std::shared_ptr<Session> getSession(const std::string& sessionId);
            //向指定会话发送消息
            bool addMessage(const std::string& sessionId, const Message& message);
            //获取指定会话的消息历史记录
            std::vector<std::shared_ptr<Message>> getHistoryMessage(const std::shared_ptr<Session> session);
            //更新会话时间戳
            void updateSessionTimestamp(const std::string& sessionId);
            //获取所有会话
            std::vector<std::string> getSessionList();
            //删除会话
            bool deleteSession(const std::string& sessionId);
            //清空所有会话
            void clearAllSessions();
            //获取会话总数
            int64_t getSessionCount();
        private:
            //生成会话id,会话id格式,session_时间戳_会话计数
            std::string generateSessionId();
            //生成消息id,消息id格式,msg_会话id_消息计数
            std::string generateMessageId(size_t messageCount);
        private:
            std::unordered_map<std::string,std::shared_ptr<Session>> _sessions;
            mutable std::mutex _mutex;
            std::atomic<int64_t> _sessionCounter = 0;
            DataManager _dataManager;
    };
}
