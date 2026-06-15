#pragma once
#include <memory>
#include <mutex>
#include <spdlog/spdlog.h>
#include <string>

namespace linne
{
    class Logger
    {
    public:
        static void
        initLogger(const std::string &loggerName, const std::string &loggerPath, spdlog::level::level_enum loggerLevel = spdlog::level::info);
        static std::shared_ptr<spdlog::logger> getLogger();

    private:
        Logger();
        Logger(const Logger &) = delete;
        Logger &operator=(const Logger &) = delete;

    private:
        static std::shared_ptr<spdlog::logger> _logger;
        static std::mutex _mutex;
    };

#define TRACE(format, ...) linne::Logger::getLogger()->trace(std::string("[{:>10s}:{:<4d}]") + fomat, __FILE__, __LINE__, __VA_ARGS__)
#define DBG(format, ...) linne::Logger::getLogger()->debug(std::string("[{:>10s}:{:<4d}]") + fomat, __FILE__, __LINE__, __VA_ARGS__)
#define INFO(format, ...) linne::Logger::getLogger()->info(std::string("[{:>10s}:{:<4d}]") + fomat, __FILE__, __LINE__, __VA_ARGS__)
#define WARN(format, ...) linne::Logger::getLogger()->warn(std::string("[{:>10s}:{:<4d}]") + fomat, __FILE__, __LINE__, __VA_ARGS__)
#define ERR(format, ...) linne::Logger::getLogger()->error(std::string("[{:>10s}:{:<4d}]") + fomat, __FILE__, __LINE__, __VA_ARGS__)
#define CRIT(format, ...) linne::Logger::getLogger()->critical(std::string("[{:>10s}:{:<4d}]") + fomat, __FILE__, __LINE__, __VA_ARGS__)
} // namespace linne
