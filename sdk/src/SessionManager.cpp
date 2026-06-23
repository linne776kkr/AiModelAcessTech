#include "../include/SessionManager.h"
#include "../include/util/myLog.h"

#include <sstream>
#include <iomanip>
#include <ctime>

namespace ai_chat_sdk{
    //生成会话id,会话id格式,session_时间戳_会话计数
    std::string SessionManager::generateSessionId()
    {
        //生成时间戳
        std::time_t timestamp = time(nullptr);
        //会话计数增加
        _sessionCounter.fetch_add(1);
        //生成会话id
        std::ostringstream oss;
        oss<<"_session"<<timestamp<<"_"<<std::setw(8)<<std::setfill('0')<<_sessionCounter;
        return oss.str();
    }
    //生成消息id,消息id格式,msg_会话id_消息计数
    std::string SessionManager::generateMessageId(size_t messageCount)
    {
        //生成时间戳
        std::time_t timestamp = time(nullptr);
        //消息计数增加
        messageCount++;
        //生成消息id
        std::ostringstream oss;
        oss<<"_msg"<<timestamp<<"_"<<std::setw(8)<<std::setfill('0')<<messageCount;
        return oss.str();
    }
    //创建会话,提供模型名称,返回会话ID
    std::string SessionManager::createSession(const std::string& modelName, const std::string& sessionName)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        //生成会话id
        std::string sessionId = generateSessionId();
        //创建会话
        std::shared_ptr<Session> session = std::make_shared<Session>();
        session->_sessionId = sessionId;
        session->_sessionName = sessionName;
        session->_modelName = modelName;
        session->_createTime = time(nullptr);
        session->_lastUpdateTime = time(nullptr);
        //添加会话到映射
        _sessions[sessionId] = session;
        INFO("create session, sessionId: %s, modelName: %s, sessionName: %s", sessionId.c_str(), modelName.c_str(), sessionName.c_str());
        return sessionId;
    }
    //获取指定会话
    std::shared_ptr<Session> SessionManager::getSession(const std::string& sessionId) const
    {
        std::unique_lock<std::mutex> lock(_mutex);
        //根据会话id获取会话
        auto it = _sessions.find(sessionId);
        if(it == _sessions.end())
        {
            WARN("session not found, sessionId: %s", sessionId.c_str());
            return nullptr;
        }
        return it->second;
    }
    //向指定会话发送消息
    bool SessionManager::addMessage(const std::string& sessionId, const Message& message)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        //根据会话id获取会话
        auto it = _sessions.find(sessionId);
        if(it == _sessions.end())
        {
            WARN("session not found, sessionId: %s", sessionId.c_str());
            return false;
        }
        //创建消息
        std::shared_ptr<Message> msg = std::make_shared<Message>(message);
        msg->_messageId = generateMessageId(it->second->_messages.size());
        msg->_timestamp = time(nullptr);
        //将消息添加到会话
        it->second->_messages.push_back(msg);
        //更新会话最后更新时间戳
        it->second->_lastUpdateTime = time(nullptr);
        INFO("add message to role: %s, message: %s", msg->_role.c_str(), msg->_content.c_str());
        return true;
    }
    //获取指定会话的消息历史记录
    std::vector<std::shared_ptr<Message>> SessionManager::getHistoryMessage(const std::string& sessionId) const
    {
        std::unique_lock<std::mutex> lock(_mutex);
        //根据会话id获取会话
        auto it = _sessions.find(sessionId);
        if(it == _sessions.end())
        {
            WARN("session not found, sessionId: %s", sessionId.c_str());
            return {};
        }
        //返回会话的消息历史记录
        return it->second->_messages;
    }
    //更新会话时间戳
    void SessionManager::updateSessionTimestamp(const std::string& sessionId)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        //根据会话id获取会话
        auto it = _sessions.find(sessionId);
        if(it == _sessions.end())
        {
            WARN("session not found, sessionId: %s", sessionId.c_str());
            return;
        }
        //更新会话最后更新时间戳
        it->second->_lastUpdateTime = time(nullptr);
        INFO("update session timestamp, sessionId: %s", sessionId.c_str());
        return;
    }
    //获取所有会话,会话id列表按最后更新时间戳排序
    std::vector<std::string> SessionManager::getSessionList() const
    {
        std::unique_lock<std::mutex> lock(_mutex);
        std::vector<std::pair<std::time_t,std::string>> sessionSort;
        for(auto session : _sessions)
        {
            sessionSort.push_back({session.second->_lastUpdateTime,session.first});
        }
        //按最后更新时间戳排序
        std::sort(sessionSort.begin(),sessionSort.end(),[](const auto& a,const auto& b){return a.first > b.first;});
        //返回会话id列表
        std::vector<std::string> sessionIds;
        for(auto session : sessionSort)
        {
            sessionIds.push_back(session.second);
        }
        return sessionIds;
    }
    //删除会话
    bool SessionManager::deleteSession(const std::string& sessionId)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        //根据会话id获取会话
        auto it = _sessions.find(sessionId);
        if(it == _sessions.end())
        {
            WARN("session not found, sessionId: %s", sessionId.c_str());
            return false;
        }
        //删除会话
        _sessions.erase(it);
        INFO("delete session, sessionId: %s", sessionId.c_str());
        return true;
    }
    //清空所有会话
    void SessionManager::clearAllSessions()
    {
        std::unique_lock<std::mutex> lock(_mutex);
        //清空所有会话
        _sessions.clear();
        INFO("clear all sessions");
        return;
    }
    //获取会话总数
    int64_t SessionManager::getSessionCount() const
    {
        std::unique_lock<std::mutex> lock(_mutex);
        //返回会话总数
        return _sessionCounter;
    }
}//namespace ai_chat_sdk