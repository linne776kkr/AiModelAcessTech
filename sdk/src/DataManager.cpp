#include "../include/DataManager.h"
#include "../include/util/myLog.h"

namespace ai_chat_sdk
{
    DataManager::DataManager(const std::string &dbPath):_dbPath(dbPath)
    {
        //打开数据库
        int ret = sqlite3_open(_dbPath.c_str(), &_db);
        if(ret != SQLITE_OK)
        {
            ERR("open database failed {}", sqlite3_errmsg(_db));
            return;
        }
        //初始化数据库
        if(!initDatabase())
        {
            ERR("init database tables failed");
            sqlite3_close(_db);
            _db = nullptr;
            return;
        }
        INFO("init database {} success", _dbPath);
    }
    DataManager::~DataManager()
    {
        if(_db)
        {
            INFO("close database {} success", _dbPath);
            sqlite3_close(_db);
            _db = nullptr;
        }
    }
    //session相关操作
    //插入新会话
    bool DataManager::insertSession(const std::shared_ptr<Session> &sessionptr)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        //构建SQL语句
        std::string insertSessionSQL = R"(
            INSERT INTO sessions (sessionId, sessionName, modelName, createTime, lastUpdateTime)
            VALUES (?, ?, ?, ?, ?)
        )";
        //准备SQL语句
        sqlite3_stmt* stmt;
        int ret = sqlite3_prepare_v2(_db, insertSessionSQL.c_str(), -1, &stmt, nullptr);
        if(ret != SQLITE_OK)
        {
            ERR("prepare insert session statement failed {}", sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            return false;
        }
        //绑定参数
        sqlite3_bind_text(stmt,1,sessionptr->_sessionId.c_str(),-1,SQLITE_STATIC);
        sqlite3_bind_text(stmt,2,sessionptr->_sessionName.c_str(),-1,SQLITE_STATIC);
        sqlite3_bind_text(stmt,3,sessionptr->_modelName.c_str(),-1,SQLITE_STATIC);
        sqlite3_bind_int64(stmt,4,static_cast<int64_t>(sessionptr->_createTime));
        sqlite3_bind_int64(stmt,5,static_cast<int64_t>(sessionptr->_lastUpdateTime));
        //执行SQL语句
        ret = sqlite3_step(stmt);
        if(ret != SQLITE_DONE)
        {
            ERR("insert session statement failed {}", sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            return false;
        }
        sqlite3_finalize(stmt);
        INFO("insert session {} success", sessionptr->_sessionId);
        return true;
    }
    //获取指定会话,没有历史消息,需要在后面联合时获取
    std::shared_ptr<Session> DataManager::getSession(const std::string &sessionId)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        //构建SQL语句
        std::string getSessionSQL = R"(
            SELECT sessionName, modelName, createTime, lastUpdateTime FROM sessions WHERE sessionId = ?
        )";
        //准备SQL语句
        sqlite3_stmt* stmt;
        int ret = sqlite3_prepare_v2(_db, getSessionSQL.c_str(), -1, &stmt, nullptr);
        if(ret != SQLITE_OK)
        {
            ERR("prepare get session statement failed {}", sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            return std::shared_ptr<Session>();
        }
        //绑定参数
        sqlite3_bind_text(stmt,1,sessionId.c_str(),-1,SQLITE_STATIC);
        //执行SQL语句
        ret = sqlite3_step(stmt);
        if(ret != SQLITE_DONE)
        {
            sqlite3_finalize(stmt);
            return nullptr;
        }
        //获取结果
        std::shared_ptr<Session> sessionptr = std::make_shared<Session>();
        sessionptr->_sessionId = sessionId;
        sessionptr->_sessionName = reinterpret_cast<const char*>(sqlite3_column_text(stmt,0));
        sessionptr->_modelName = reinterpret_cast<const char*>(sqlite3_column_text(stmt,1));
        sessionptr->_createTime = static_cast<std::time_t>(sqlite3_column_int64(stmt,2));
        sessionptr->_lastUpdateTime = static_cast<std::time_t>(sqlite3_column_int64(stmt,3));
        sqlite3_finalize(stmt);
        INFO("get session {} success", sessionId);
        return sessionptr;
    }
    //更新指定会话时间戳
    bool DataManager::updateSessionTimestamp(const std::string &sessionId,std::time_t timestamp)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        //构建SQL语句
        std::string updateSessionTimestampSQL = R"(
            UPDATE sessions SET lastUpdateTime = ? WHERE sessionId = ?
        )";
        //准备SQL语句
        sqlite3_stmt* stmt;
        int ret = sqlite3_prepare_v2(_db, updateSessionTimestampSQL.c_str(), -1, &stmt, nullptr);
        if(ret != SQLITE_OK)
        {
            ERR("prepare update session timestamp statement failed {}", sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            return false;
        }
        //绑定参数
        sqlite3_bind_int64(stmt,1,static_cast<int64_t>(timestamp));
        sqlite3_bind_text(stmt,2,sessionId.c_str(),-1,SQLITE_STATIC);
        //执行SQL语句
        ret = sqlite3_step(stmt);
        if(ret != SQLITE_DONE)
        {
            ERR("update session timestamp statement failed {}", sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            return false;
        }
        sqlite3_finalize(stmt);
        INFO("update session {} {} success", sessionId, timestamp);
        return true;
    }
    //更新会话名称
    bool DataManager::updateSessionName(const std::string &sessionId,const std::string &sessionName)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        //构建SQL语句
        std::string updateSessionNameSQL = R"(
            UPDATE sessions SET sessionName = ? WHERE sessionId = ?
        )";
        //准备SQL语句
        sqlite3_stmt* stmt;
        int ret = sqlite3_prepare_v2(_db, updateSessionNameSQL.c_str(), -1, &stmt, nullptr);
        if(ret != SQLITE_OK)
        {
            ERR("prepare update session name statement failed {}", sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            return false;
        }
        //绑定参数
        sqlite3_bind_text(stmt,1,sessionName.c_str(),-1,SQLITE_STATIC);
        sqlite3_bind_text(stmt,2,sessionId.c_str(),-1,SQLITE_STATIC);
        //执行SQL语句
        ret = sqlite3_step(stmt);
        if(ret != SQLITE_DONE)
        {
            ERR("update session name statement failed {}", sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            return false;
        }
        sqlite3_finalize(stmt);
        INFO("update session {} {} success", sessionId, sessionName);
        return true;
    }
    //删除指定会话
    bool DataManager::deleteSession(const std::string &sessionId)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        //构建SQL语句
        std::string deleteSessionSQL = R"(
            DELETE FROM sessions WHERE sessionId = ?
        )";
        //准备SQL语句
        sqlite3_stmt* stmt;
        int ret = sqlite3_prepare_v2(_db, deleteSessionSQL.c_str(), -1, &stmt, nullptr);
        if(ret != SQLITE_OK)
        {
            ERR("prepare delete session statement failed {}", sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            return false;
        }
        //绑定参数
        sqlite3_bind_text(stmt,1,sessionId.c_str(),-1,SQLITE_STATIC);
        //执行SQL语句
        ret = sqlite3_step(stmt);
        if(ret != SQLITE_DONE)
        {
            ERR("delete session statement failed {}", sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            return false;
        }
        sqlite3_finalize(stmt);
        INFO("delete session {} success", sessionId);
        return true;
    }
    //获取所有会话id
    std::vector<std::string> DataManager::getAllSessionId()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        //构建SQL语句
        std::string getAllSessionIdSQL = R"(
            SELECT sessionId FROM sessions ORDER BY lastUpdateTime DESC
        )";
        //准备SQL语句
        sqlite3_stmt* stmt;
        int ret = sqlite3_prepare_v2(_db, getAllSessionIdSQL.c_str(), -1, &stmt, nullptr);
        if(ret != SQLITE_OK)
        {
            ERR("prepare get all session id statement failed {}", sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            return {};
        }
        //执行SQL语句
        std::vector<std::string> sessionIds;
        while((ret = sqlite3_step(stmt)) == SQLITE_ROW)
        {
            sessionIds.push_back(reinterpret_cast<const char*>(sqlite3_column_text(stmt,0)));
        }
        if(ret != SQLITE_DONE)
        {
            ERR("get all session id statement failed {}", sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            return {};
        }
        INFO("get all session id success, count: {}", sessionIds.size());
        sqlite3_finalize(stmt);
        return sessionIds;
    }
    //获取所有会话信息,历史信息字段后续添加
    std::vector<std::shared_ptr<Session>> DataManager::getAllSession()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        //构建SQL语句
        std::string getAllSessionSQL = R"(
            SELECT sessionId,sessionName,modelName,createTime,lastUpdateTime FROM sessions ORDER BY lastUpdateTime DESC
        )";
        //准备SQL语句
        sqlite3_stmt* stmt;
        int ret = sqlite3_prepare_v2(_db, getAllSessionSQL.c_str(), -1, &stmt, nullptr);
        if(ret != SQLITE_OK)
        {
            ERR("prepare get all session statement failed {}", sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            return {};
        }
        //执行SQL语句
        std::vector<std::shared_ptr<Session>> sessions;
        while((ret = sqlite3_step(stmt)) == SQLITE_ROW)
        {
            std::shared_ptr<Session> session = std::make_shared<Session>();
            session->_sessionId = reinterpret_cast<const char*>(sqlite3_column_text(stmt,0));
            session->_sessionName = reinterpret_cast<const char*>(sqlite3_column_text(stmt,1));
            session->_modelName = reinterpret_cast<const char*>(sqlite3_column_text(stmt,2));
            session->_createTime = static_cast<std::time_t>(sqlite3_column_int(stmt,3));
            session->_lastUpdateTime = static_cast<std::time_t>(sqlite3_column_int(stmt,4));
            sessions.push_back(session);
        }
        if(ret != SQLITE_DONE)
        {
            ERR("get all session statement failed {}", sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            return {};
        }
        INFO("get all session success, count: {}", sessions.size());
        sqlite3_finalize(stmt);
        return sessions;
    }
    //获取会话总数
    int64_t DataManager::getSessionCount()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        //构建SQL语句
        std::string getSessionCountSQL = R"(
            SELECT COUNT(*) FROM sessions
        )";
        //准备SQL语句
        sqlite3_stmt* stmt;
        int ret = sqlite3_prepare_v2(_db, getSessionCountSQL.c_str(), -1, &stmt, nullptr);
        if(ret != SQLITE_OK)
        {
            ERR("prepare get session count statement failed {}", sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            return 0;
        }
        //执行SQL语句
        ret = sqlite3_step(stmt);
        if(ret != SQLITE_ROW)
        {
            ERR("get session count statement failed {}", sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            return 0;
        }
        int64_t sessionCount = sqlite3_column_int64(stmt,0);
        INFO("get session count success, count: {}", sessionCount);
        sqlite3_finalize(stmt);
        return sessionCount;
    }
    //message相关操作
    //插入新消息,同时会话更新时间戳
    bool DataManager::insertMessage(const std::string& sessionId, const std::shared_ptr<Message>& message)
    {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            //构建SQL语句
            std::string insertMessageSQL = R"(
                INSERT INTO messages (messageId,sessionId,role,content,timestamp)
            VALUES (?,?,?,?,?)
            )";
            //准备SQL语句
            sqlite3_stmt* stmt;
            int ret = sqlite3_prepare_v2(_db, insertMessageSQL.c_str(), -1, &stmt, nullptr);
            if(ret != SQLITE_OK)
            {
                ERR("prepare insert message statement failed {}", sqlite3_errmsg(_db));
                sqlite3_finalize(stmt);
                return false;
            }
            //绑定参数
            sqlite3_bind_text(stmt,1,message->_messageId.c_str(),-1,SQLITE_STATIC);
            sqlite3_bind_text(stmt,2,sessionId.c_str(),-1,SQLITE_STATIC);
            sqlite3_bind_text(stmt,3,message->_role.c_str(),-1,SQLITE_STATIC);
            sqlite3_bind_text(stmt,4,message->_content.c_str(),-1,SQLITE_STATIC);
            sqlite3_bind_int(stmt,5,static_cast<int64_t>(message->_timestamp));
            //执行SQL语句
            ret = sqlite3_step(stmt);
            if(ret != SQLITE_DONE)
            {
                ERR("insert message statement failed {}", sqlite3_errmsg(_db));
                sqlite3_finalize(stmt);
                return false;
            }
            INFO("insert message success");
            sqlite3_finalize(stmt);
        }
        updateSessionTimestamp(sessionId,message->_timestamp);
        return true;
    }
    //获取指定会话所有消息
    std::vector<std::shared_ptr<Message>> DataManager::getSessionMessages(const std::string& sessionId)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        //构建SQL语句
        std::string getSessionMessagesSQL = R"(
            SELECT messageId,sessionId,role,content,timestamp FROM messages WHERE sessionId = ?
        )";
        //准备SQL语句
        sqlite3_stmt* stmt;
        int ret = sqlite3_prepare_v2(_db, getSessionMessagesSQL.c_str(), -1, &stmt, nullptr);
        if(ret != SQLITE_OK)
        {
            ERR("prepare get session messages statement failed {}", sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            return {};
        }
        //绑定参数
        sqlite3_bind_text(stmt,1,sessionId.c_str(),-1,SQLITE_STATIC);
        //执行SQL语句
        std::vector<std::shared_ptr<Message>> messages;
        while((ret = sqlite3_step(stmt)) == SQLITE_ROW)
        {
            std::shared_ptr<Message> message = std::make_shared<Message>();
            message->_messageId = reinterpret_cast<const char*>(sqlite3_column_text(stmt,0));
            message->_role = reinterpret_cast<const char*>(sqlite3_column_text(stmt,2));
            message->_content = reinterpret_cast<const char*>(sqlite3_column_text(stmt,3));
            message->_timestamp = static_cast<std::time_t>(sqlite3_column_int64(stmt,4));
            messages.push_back(message);
        }
        if(ret != SQLITE_DONE)
        {
            ERR("get session messages statement failed {}", sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            return {};
        }
        INFO("get session messages success, count: {}", messages.size());
        sqlite3_finalize(stmt);
        return messages;
    }   
    //删除指定会话的所有消息
    bool DataManager::deleteSessionMessages(const std::string& sessionId)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        //构建SQL语句
        std::string deleteSessionMessagesSQL = R"(
            DELETE FROM messages WHERE sessionId = ?
        )";
        //准备SQL语句
        sqlite3_stmt* stmt;
        int ret = sqlite3_prepare_v2(_db, deleteSessionMessagesSQL.c_str(), -1, &stmt, nullptr);
        if(ret != SQLITE_OK)
        {
            ERR("prepare delete session messages statement failed {}", sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            return false;
        }
        //绑定参数
        sqlite3_bind_text(stmt,1,sessionId.c_str(),-1,SQLITE_STATIC);
        //执行SQL语句
        ret = sqlite3_step(stmt);
        if(ret != SQLITE_DONE)
        {
            ERR("delete session messages statement failed {}", sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            return false;
        }
        INFO("delete session messages success");
        sqlite3_finalize(stmt);
        return true;
    }
    //删除会话表
    bool DataManager::deleteAllSessions()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        //构建SQL语句
        std::string deleteAllSessionsSQL = R"(
            DELETE FROM sessions
        )";
        //准备SQL语句
        sqlite3_stmt* stmt;
        int ret = sqlite3_prepare_v2(_db, deleteAllSessionsSQL.c_str(), -1, &stmt, nullptr);
        if(ret != SQLITE_OK)
        {
            ERR("prepare delete all sessions statement failed {}", sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            return false;
        }
        //执行SQL语句
        ret = sqlite3_step(stmt);
        if(ret != SQLITE_DONE)
        {
            ERR("delete all sessions statement failed {}", sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            return false;
        }
        INFO("delete all sessions success");
        sqlite3_finalize(stmt);
        return true;
    }


    //类内私有成员函数
    //初始化数据库--创建数据库表
    bool DataManager::initDatabase()
    {
        executeSQL("PRAGMA foreign_keys = ON;");
        //创建会话表
        //sessions sessionId(string,primary key),sessionName(string),modelName(string),createTime(int),lastUpdateTime(int)
        std::string createSessionTablesSql = R"(
            CREATE TABLE IF NOT EXISTS sessions (
                sessionId TEXT PRIMARY KEY,
                sessionName TEXT DEFAULT '',
                modelName TEXT NOT NULL,
                createTime INTEGER NOT NULL,
                lastUpdateTime INTEGER NOT NULL
            );
        )";
        if(!executeSQL(createSessionTablesSql))
        {
            return false;
        }
        //创建信息表
        //messages messageId(string,primary key),sessionId(string,foreign key sessions(sessionId)),role(string),content(string),timestamp(int)
        std::string createMessageTablesSql = R"(
            CREATE TABLE IF NOT EXISTS messages (
                messageId TEXT PRIMARY KEY,
                sessionId TEXT NOT NULL,
                role TEXT NOT NULL,
                content TEXT NOT NULL,
                timestamp INTEGER NOT NULL,
                FOREIGN KEY (sessionId) REFERENCES sessions (sessionId) ON DELETE CASCADE
            );
        )";
        if(!executeSQL(createMessageTablesSql))
        {
            return false;
        }
        return true;
    }
    //执行SQL语句的工具函数
    bool DataManager::executeSQL(const std::string &sql)
    {
        if(!_db)
        {
            ERR("database not open");
            return false;
        }
        char* errMsg = nullptr;
        int ret = sqlite3_exec(_db, sql.c_str(), nullptr, nullptr, &errMsg);
        if(ret != SQLITE_OK)
        {
            ERR("execute sql failed {}", errMsg);
            sqlite3_free(errMsg);
            return false;
        }
        return true;
    }
}
