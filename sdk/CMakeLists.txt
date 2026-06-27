#设置cmake的最小版本
cmake_minimum_required(VERSION 3.10)
#设置SDK的名称
set(SDK_NAME "ai_chat_sdk")
#设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#设置构建类型Release
set(CMAKE_BUILD_TYPE Release)
#收集源文件和头文件
file(GLOB_RECURSE SDK_SOURCES "src/*.cpp")
file(GLOB_RECURSE SDK_HEADERS "include/*.h")
#生成静态库
add_library(${SDK_NAME} STATIC ${SDK_SOURCES} ${SDK_HEADERS})
#设置输出目录
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build)
#添加CPPHTTPLIB_OPENSSL_SUPPORT宏定义
target_compile_definitions(${SDK_NAME} PUBLIC CPPHTTPLIB_OPENSSL_SUPPORT)
#添加头文件搜索路径
target_include_directories(${SDK_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})
#设置静态库的目录
link_directories(/usr/local/lib)
#添加链接库
target_link_libraries(${SDK_NAME} jsoncpp fmt spdlog OpenSSL::SSL OpenSSL::Crypto sqlite3)
#安装规则 拷贝静态库到系统目录下
install(TARGETS ${SDK_NAME} ARCHIVE DESTINATION lib)
#安装规则拷贝头文件到系统目录下/usr/Local/incLude
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
DESTINATION include/ai_chat_sdk
FILES_MATCHING PATTERN "*.h")
