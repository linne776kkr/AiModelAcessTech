#设置cmake的最小版本
cmake_minimum_required(VERSION 3.10)
#设置项目名称
project(testLLM)
#设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#设置构建类型Debeg
set(CMAKE_BUILD_TYPE Debug)
#添加可执行文件
add_executable(testLLM LLMtest.cpp
../sdk/src/util/myLog.cpp
../sdk/src/DeepSeekProvider.cpp
../sdk/src/ChatGPTProvider.cpp)
#设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
#添加头文件搜索路径
include_directories(/../sdk/include)

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})
#添加CPPHTTPLIB_OPENSSL_SUPPORT宏定义
target_compile_definitions(testLLM PRIVATE CPPHTTPLIB_OPENSSL_SUPPORT)
#连接库
target_link_libraries(testLLM jsoncpp fmt spdlog gtest OpenSSL::SSL OpenSSL::Crypto)