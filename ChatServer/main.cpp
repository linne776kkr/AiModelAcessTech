#include "ChatServer.h"
#include <atomic>
#include <thread>
#include <iostream>
// 引入gflags库，用于命令行参数解析
#include <gflags/gflags.h>
// 引入文件流，用于读取和生成配置文件
#include <fstream>
// 引入信号处理库，用于捕获Ctrl+C等系统信号
#include <csignal>


// 定义服务器版本号常量
#define VERSION "1.0.0"

// 使用gflags定义命令行参数，格式：DEFINE_类型(参数名, 默认值, 参数说明)
// 服务器绑定地址，默认监听所有网络接口
DEFINE_string(host, "0.0.0.0", "服务器绑定地址");
// 服务器绑定端口，默认8080
DEFINE_int32(port, 8080, "服务器绑定端口");
// 日志级别，支持DEBUG/INFO/WARN/ERROR四个级别，默认INFO
DEFINE_string(log_level, "INFO", "日志级别(DEBUG/INFO/WARN/ERROR)");
// 温度参数，控制AI回复的随机性，范围0~2，默认0.7
DEFINE_double(temperature, 0.7, "温度参数(0~2)");
// 最大token数，限制单次对话的token上限，默认2048
DEFINE_int32(max_tokens, 2048, "最大token数");
// Ollama模型名称，使用本地Ollama时需要配置
DEFINE_string(ollama_model_name, "", "Ollama模型名称");
// Ollama模型描述，使用本地Ollama时需要配置
DEFINE_string(ollama_model_desc, "", "Ollama模型描述");
// Ollama服务地址，使用本地Ollama时需要配置
DEFINE_string(ollama_endpoint, "", "Ollama服务地址");

// 全局原子变量，用于标记服务器是否正在运行，支持多线程安全访问
std::atomic<bool> g_running{true};

// 信号处理函数，用于捕获SIGINT(Ctrl+C)和SIGTERM(终止信号)
void signalHandler(int signal) {
    // 判断是否为需要处理的信号类型
    if (signal == SIGINT || signal == SIGTERM) {
        // 将运行标志置为false，通知主循环退出
        g_running.store(false);
    }
}

// 生成默认配置文件函数，当配置文件不存在时自动创建
void generateDefaultConfig(const std::string& configPath) {
    // 打开配置文件进行写入
    std::ofstream ofs(configPath);
    // 检查文件是否成功打开
    if (!ofs) {
        std::cerr << "无法创建配置文件: " << configPath << std::endl;
        return;
    }
    // 写入配置文件头部注释
    ofs << "# ChatServer 配置文件\n";
    // 写入服务器配置部分
    ofs << "# 服务器配置\n";
    ofs << "--host=0.0.0.0\n";
    ofs << "--port=8080\n";
    ofs << "--log_level=INFO\n";
    ofs << "\n";
    // 写入模型配置部分
    ofs << "# 模型配置\n";
    ofs << "--temperature=0.7\n";
    ofs << "--max_tokens=2048\n";
    ofs << "\n";
    // 写入Ollama配置部分
    ofs << "# Ollama配置(使用Ollama时填写)\n";
    ofs << "--ollama_model_name=\n";
    ofs << "--ollama_model_desc=\n";
    ofs << "--ollama_endpoint=\n";
    // 关闭文件流
    ofs.close();
    // 输出提示信息
    std::cout << "已生成默认配置文件: " << configPath << std::endl;
}

// 打印帮助信息函数，显示服务器的使用方法、选项和接口说明
void printHelp() {
    // 输出服务器名称和版本号
    std::cout << "\nAIChatServer v" << VERSION << " - AI聊天服务器\n\n";
    // 输出用法说明
    std::cout << "用法:\n";
    std::cout << "  ./AIChatServer [选项]\n";
    std::cout << "  ./AIChatServer -flagfile=ChatServer.conf\n\n";
    // 输出选项列表
    std::cout << "选项:\n";
    std::cout << "  -host              服务器绑定地址 (默认: 0.0.0.0)\n";
    std::cout << "  -port              服务器绑定端口 (默认: 8080)\n";
    std::cout << "  -log_level         日志级别(DEBUG/INFO/WARN/ERROR) (默认: INFO)\n";
    std::cout << "  -temperature       温度参数(0~2) (默认: 0.7)\n";
    std::cout << "  -max_tokens        最大token数 (默认: 2048)\n";
    std::cout << "  -ollama_model_name Ollama模型名称\n";
    std::cout << "  -ollama_model_desc Ollama模型描述\n";
    std::cout << "  -ollama_endpoint   Ollama服务地址\n";
    std::cout << "  -flagfile          从配置文件加载参数\n";
    std::cout << "  -h, --help         显示帮助信息\n";
    std::cout << "  -v, --version      显示版本号\n\n";
    // 输出环境变量说明
    std::cout << "环境变量:\n";
    std::cout << "  DEEP_SEEK_API_KEY  DeepSeek API密钥\n";
    std::cout << "  OPENAI_API_KEY     OpenAI API密钥\n\n";
    // 输出API接口说明
    std::cout << "接口说明:\n";
    std::cout << "  POST   /api/session          创建会话\n";
    std::cout << "  GET    /api/sessions         获取会话列表\n";
    std::cout << "  GET    /api/models           获取模型列表\n";
    std::cout << "  DELETE /api/session/{id}     删除会话\n";
    std::cout << "  GET    /api/session/{id}/history  获取历史消息\n";
    std::cout << "  POST   /api/message          发送消息(全量返回)\n";
    std::cout << "  POST   /api/message/async    发送消息(流式返回)\n\n";
    // 输出使用示例
    std::cout << "使用示例:\n";
    std::cout << "  ./AIChatServer -port=8081 -temperature=0.9\n";
    std::cout << "  ./AIChatServer -flagfile=ChatServer.conf\n";
    std::cout << "  export DEEP_SEEK_API_KEY=your_key_here && ./AIChatServer\n";
    std::cout << std::endl;
}

// 打印版本号函数
void printVersion() {
    std::cout << "AIChatServer v" << VERSION << std::endl;
}

// 配置参数验证函数，检查配置是否合法
bool validateConfig(const ai_chat_server::ChatServerConfig& config) {
    // 验证温度参数范围
    if (config.temperature < 0 || config.temperature > 2) {
        std::cerr << "错误: 温度参数必须在0~2之间, 当前值: " << config.temperature << std::endl;
        return false;
    }
    // 验证最大token数必须大于0
    if (config.maxTokens <= 0) {
        std::cerr << "错误: 最大token数必须大于0, 当前值: " << config.maxTokens << std::endl;
        return false;
    }
    // 检查是否配置了至少一个API密钥
    bool hasApiKey = !config.deepSeekAPIKey.empty() || !config.chatGPTAPIKey.empty();
    // 检查是否配置了完整的Ollama参数
    bool hasOllama = !config.ollamaModelName.empty() && 
                      !config.ollamaModelDesc.empty() && 
                      !config.ollamaEndpoint.empty();
    // 验证至少需要配置一个API密钥或完整的Ollama配置
    if (!hasApiKey && !hasOllama) {
        std::cerr << "错误: 至少需要配置一个API密钥或完整的Ollama配置" << std::endl;
        return false;
    }
    // 验证Ollama配置参数完整性
    if (hasOllama && (config.ollamaModelName.empty() || 
                      config.ollamaModelDesc.empty() || 
                      config.ollamaEndpoint.empty())) {
        std::cerr << "错误: Ollama配置参数不能为空" << std::endl;
        return false;
    }
    // 验证端口号范围
    if (config.port <= 0 || config.port > 65535) {
        std::cerr << "错误: 端口号必须在1~65535之间, 当前值: " << config.port << std::endl;
        return false;
    }
    // 所有验证通过
    return true;
}

// 主函数，程序入口点
int main(int argc, char* argv[]) {
    // 第一步：处理版本和帮助命令行参数（在gflags解析之前处理）
    // 遍历所有命令行参数
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        // 如果是版本参数，打印版本并退出
        if (arg == "-v" || arg == "--version") {
            printVersion();
            return 0;
        }
        // 如果是帮助参数，打印帮助信息并退出
        if (arg == "-h" || arg == "--help") {
            printHelp();
            return 0;
        }
    }

    // 第二步：检查并生成默认配置文件
    // 获取可执行文件所在目录，用于定位配置文件
    std::string exePath = argv[0];
    std::string configPath;
    size_t lastSlash = exePath.find_last_of('/');
    if (lastSlash != std::string::npos) {
        configPath = exePath.substr(0, lastSlash + 1) + "ChatServer.conf";
    } else {
        configPath = "ChatServer.conf";
    }
    // 尝试打开配置文件
    std::ifstream configFile(configPath);
    // 如果配置文件不存在，生成默认配置文件
    if (!configFile) {
        generateDefaultConfig(configPath);
    }

    // 第三步：检查是否已传入-flagfile参数，如果没有则自动添加配置文件路径
    bool hasFlagFile = false;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg.find("-flagfile") != std::string::npos || arg.find("--flagfile") != std::string::npos) {
            hasFlagFile = true;
            break;
        }
    }
    // 如果没有传入-flagfile参数，自动添加配置文件路径
    char** originalArgv = nullptr;
    if (!hasFlagFile) {
        int newArgc = argc + 2;
        originalArgv = new char*[newArgc];
        for (int i = 0; i < argc; ++i) {
            originalArgv[i] = argv[i];
        }
        originalArgv[argc] = const_cast<char*>("-flagfile");
        originalArgv[argc + 1] = const_cast<char*>(configPath.c_str());
        argc = newArgc;
        argv = originalArgv;
    }

    // 第四步：使用gflags解析命令行参数
    // 第三个参数true表示移除已解析的参数，使argv只保留未解析的参数
    google::ParseCommandLineFlags(&argc, &argv, true);

    // 释放动态分配的内存
    if (!hasFlagFile && originalArgv != nullptr) {
        delete[] originalArgv;
    }

    // 第五步：构建服务器配置对象
    // 创建配置对象实例
    ai_chat_server::ChatServerConfig config;
    // 从gflags解析结果中读取配置参数
    config.host = FLAGS_host;           // 服务器绑定地址
    config.port = FLAGS_port;           // 服务器绑定端口
    config.logLevel = FLAGS_log_level;  // 日志级别
    config.temperature = FLAGS_temperature;  // 温度参数
    config.maxTokens = FLAGS_max_tokens;     // 最大token数
    config.ollamaModelName = FLAGS_ollama_model_name;  // Ollama模型名称
    config.ollamaModelDesc = FLAGS_ollama_model_desc;  // Ollama模型描述
    config.ollamaEndpoint = FLAGS_ollama_endpoint;     // Ollama服务地址

    // 第六步：从环境变量中读取API密钥
    // 读取DeepSeek API密钥环境变量
    const char* deepSeekKey = std::getenv("DEEP_SEEK_API_KEY");
    // 读取OpenAI API密钥环境变量
    const char* chatGPTKey = std::getenv("OPENAI_API_KEY");
    // 如果环境变量存在，赋值给配置对象
    if (deepSeekKey) {
        config.deepSeekAPIKey = deepSeekKey;
    }
    if (chatGPTKey) {
        config.chatGPTAPIKey = chatGPTKey;
    }

    // 第七步：验证配置参数合法性
    // 调用验证函数检查配置是否符合要求
    if (!validateConfig(config)) {
        // 验证失败，返回错误码1
        return 1;
    }

    // 第八步：打印服务器配置信息
    // 输出配置摘要，方便用户确认当前配置
    std::cout << "服务器配置:\n";
    std::cout << "  地址: " << config.host << ":" << config.port << std::endl;
    std::cout << "  日志级别: " << config.logLevel << std::endl;
    std::cout << "  温度参数: " << config.temperature << std::endl;
    std::cout << "  最大token数: " << config.maxTokens << std::endl;
    std::cout << "  DeepSeek API Key: " << (config.deepSeekAPIKey.empty() ? "未配置" : "已配置") << std::endl;
    std::cout << "  ChatGPT API Key: " << (config.chatGPTAPIKey.empty() ? "未配置" : "已配置") << std::endl;
    std::cout << "  Ollama模型: " << (config.ollamaModelName.empty() ? "未配置" : config.ollamaModelName) << std::endl;

    // 第九步：注册信号处理函数
    // 注册SIGINT信号处理器（Ctrl+C）
    signal(SIGINT, signalHandler);
    // 注册SIGTERM信号处理器（kill命令）
    signal(SIGTERM, signalHandler);

    // 第十步：初始化日志系统
    // 初始化spdlog日志，设置日志名称为ChatServer，输出到标准输出，日志级别为INFO
    linne::Logger::initLogger("ChatServer", "stdout", spdlog::level::info);

    // 第十一步：创建ChatServer实例
    // 使用配置对象初始化服务器实例
    ai_chat_server::ChatServer server(config);

    // 第十二步：启动服务器
    // 调用start方法启动HTTP服务器
    if (!server.start()) {
        // 启动失败，输出错误信息并返回错误码1
        std::cerr << "服务器启动失败" << std::endl;
        return 1;
    }

    // 第十三步：输出启动成功提示信息
    std::cout << "\n服务器已启动, 按 Ctrl+C 退出" << std::endl;

    // 第十四步：主循环，等待退出信号
    // 循环条件：全局运行标志为true且服务器正在运行
    while (g_running.load() && server.isRunning()) {
        // 每秒检查一次，避免CPU空转
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // 第十五步：停止服务器
    // 调用stop方法优雅停止服务器，释放资源
    server.stop();
    // 输出停止提示信息
    std::cout << "\n服务器已停止" << std::endl;

    // 程序正常退出，返回0
    return 0;
}