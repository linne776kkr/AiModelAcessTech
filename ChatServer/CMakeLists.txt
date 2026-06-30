# 设置CMake最低版本要求为3.10，确保支持所需的CMake特性
cmake_minimum_required(VERSION 3.10)

# 定义项目名称为AIChatServer
set(PROJECT_NAME "AIChatServer")
# 使用project命令初始化项目，指定项目名称
project(${PROJECT_NAME})

# 设置C++标准版本为C++17，确保支持现代C++特性（如智能指针、lambda表达式等）
set(CMAKE_CXX_STANDARD 17)
# 强制要求编译器必须支持指定的C++标准
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# 设置构建类型为Release，启用编译器优化，适合生产环境
set(CMAKE_BUILD_TYPE Release)

# 使用file命令递归查找当前目录下所有.cpp文件，存储到SERVER_SOURCES变量中
file(GLOB_RECURSE SERVER_SOURCES "*.cpp")
# 使用list命令过滤掉SERVER_SOURCES中匹配CMakeFiles目录的文件，避免包含CMake自动生成的文件
list(FILTER SERVER_SOURCES EXCLUDE REGEX "CMakeFiles")

# 使用add_executable命令创建可执行文件，目标名称为项目名称，源文件为SERVER_SOURCES变量中的所有文件
add_executable(${PROJECT_NAME} ${SERVER_SOURCES})

# 使用target_compile_definitions命令为目标添加编译宏定义，启用cpp-httplib的OpenSSL支持，用于HTTPS通信
target_compile_definitions(${PROJECT_NAME} PUBLIC CPPHTTPLIB_OPENSSL_SUPPORT)

# 使用target_include_directories命令设置目标的头文件搜索路径
# ${CMAKE_CURRENT_SOURCE_DIR}：当前CMakeLists.txt所在的目录
# /usr/local/include：系统级别的第三方库头文件目录
target_include_directories(${PROJECT_NAME} PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    /usr/local/include
)

# 使用find_package命令查找OpenSSL库，REQUIRED参数表示如果找不到则报错
find_package(OpenSSL REQUIRED)
# 使用include_directories命令添加OpenSSL的头文件搜索路径
include_directories(${OPENSSL_INCLUDE_DIR})

# 使用link_directories命令添加库文件搜索路径，指定/usr/local/lib目录
link_directories(/usr/local/lib)

# 使用target_link_libraries命令为目标链接所需的库文件
target_link_libraries(${PROJECT_NAME} 
    ai_chat_sdk        # ChatSDK静态库，提供AI聊天核心功能
    jsoncpp            # JSON解析库，用于处理API请求和响应的JSON数据
    fmt                # 格式化库，用于字符串格式化
    spdlog             # 日志库，用于程序日志记录
    gflags             # 命令行参数解析库，用于解析命令行参数
    OpenSSL::SSL       # OpenSSL SSL库，用于HTTPS通信
    OpenSSL::Crypto    # OpenSSL加密库，用于加密操作
    sqlite3            # SQLite数据库库，用于会话和消息的持久化存储
    pthread            # POSIX线程库，用于多线程支持
)

# 使用set命令设置可执行文件的输出路径为当前目录下的build文件夹
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build)

# 使用install命令定义安装规则，将生成的可执行文件安装到系统的bin目录
install(TARGETS ${PROJECT_NAME} DESTINATION bin)