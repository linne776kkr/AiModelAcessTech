#pragma once
#include "../../include/util/myLog.h"
#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace linne {
std::shared_ptr<spdlog::logger> Logger::_logger = nullptr;
std::mutex Logger::_mutex;

Logger::Logger(){};
void Logger::initLogger(const std::string &loggerName,const std::string &loggerPath,spdlog::level::level_enum loggerLevel) {
    if (_logger.get() == nullptr) {
        std::unique_lock<std::mutex> lock(_mutex);
        if (_logger.get() == nullptr) {
        // 1.设置全局自动刷新级别,当日志等级>=logLevel时,自动刷新日志到文件
        spdlog::flush_on(loggerLevel);
        // 2.启动异步日志,将日志信息存放在队列中,由线程池负责写入
        spdlog::init_thread_pool(32768, 1);
        // 3.创建日志记录器
        if (loggerPath == "stdout") {
            // 创建一个带颜色的输出到控制台的日志器
            _logger = spdlog::stdout_color_mt(loggerName);
        } 
        else {
            // 创建一个日志器,将日志写入到指定文件
            _logger = spdlog::basic_logger_mt(loggerName, loggerPath);
        }   
    }
  }
}
} // namespace linne
