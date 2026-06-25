#pragma once
#include<sqlite3.h>
#include<string>
#include<mutex>
#include<memory>
#include"common.h"
namespace ai_chat_sdk
{
    class DataManager
    {
        public:
            DataManager(const std::string &dbPath);
            ~DataManager();
            //session相关操作
            //插入新会话
            bool insertSession(const std::shared_ptr<Session> &sessionptr);
            //获取指定会话
            std::shared_ptr<Session> getSession(const std::string &sessionId);
            //更新指定会话时间戳
            bool updateSessionTimestamp(const std::string &sessionId,std::time_t timestamp);
            //更新会话名称
            bool updateSessionName(const std::string &sessionId,const std::string &sessionName);
            //删除指定会话
            bool deleteSession(const std::string &sessionId);
            //获取所有会话id
            std::vector<std::string> getAllSessionId();
            //获取所有会话信息
            std::vector<std::shared_ptr<Session>> getAllSession();
            //获取会话总数
            int64_t getSessionCount();
            //message相关操作
            //插入新消息
            bool insertMessage(const std::string& sessionId, const std::shared_ptr<Message>& message);
            //获取指定会话所有消息
            std::vector<std::shared_ptr<Message>> getSessionMessages(const std::string& sessionId);
            //删除指定会话的所有消息
            bool deleteSessionMessages(const std::string& sessionId);
        private:
            //初始化数据库--创建数据库表
            bool initDatabase();
            //执行SQL语句的工具函数
            bool executeSQL(const std::string &sql);
        private:
            sqlite3 *_db = nullptr;  // 数据库句柄
            std::string _dbPath;     // 数据库路径
            std::mutex _mutex;       // 数据库操作互斥锁
    };
}